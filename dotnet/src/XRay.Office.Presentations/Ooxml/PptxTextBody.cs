using System.Globalization;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Numbering;
using XRay.Office.Core.Units;
using XRay.Office.Ooxml.DrawingML;
using XRay.Office.Text.Fonts;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Layout;

namespace XRay.Office.Presentations.Ooxml;

/// <summary>
/// Reads a DrawingML text body into the paragraphs slide layout takes.
/// </summary>
/// <remarks>
/// <para>
/// A second reading of the same <c>a:p</c>/<c>a:r</c> tree
/// <see cref="XRay.Office.Ooxml.DrawingML.DrawingTextBody"/> extracts from, keeping what extraction
/// throws away: the size, the typeface, the colour, the alignment and the spacing. Sharing one
/// walk would mean the content tree carrying a font size, which is a rendering property that
/// nothing indexing text has any use for.
/// </para>
/// <para>
/// <strong>The whole inheritance chain is resolved, per level.</strong> A run's own
/// <c>a:rPr</c>, then the paragraph's <c>a:defRPr</c>, then the body's own <c>a:lstStyle</c>
/// entry for the paragraph's level, and then whatever the caller supplies — the layout
/// placeholder's list style, the master placeholder's, the master's <c>p:txStyles</c> and the
/// presentation's <c>p:defaultTextStyle</c>. <see cref="PptxTextStyles"/> owns that tail, so
/// extraction and rendering resolve one chain rather than two.
/// </para>
/// <para>
/// It is not an optional refinement. A PowerPoint-authored deck states its bullets, its
/// per-level indents and often its sizes <em>once</em>, on the master, and never on the slide —
/// so a reader that stops at the body draws an unindented, unbulleted outline and loses one
/// word per bulleted line to the missing marker.
/// </para>
/// </remarks>
internal static class PptxTextBody
{
    /// <summary>DrawingML's default run size when nothing in the chain states one: 18 pt.</summary>
    private const int DefaultSizeHundredthsOfPoint = 1800;

    /// <summary>
    /// <c>WINDOWS_CHARSET_SYMBOL</c>, the only <c>a:sym/@charset</c> value that makes the
    /// request symbol-encoded — <c>oox/inc/drawingml/textfont.hxx</c> and
    /// <c>TextFont::implGetFontData</c>. Absent means <c>WINDOWS_CHARSET_DEFAULT</c>, which is
    /// 1 and not this.
    /// </summary>
    private const int WindowsCharsetSymbol = 2;

    /// <summary>
    /// The character an <c>a:br</c> becomes.
    /// </summary>
    /// <remarks>
    /// U+2028, whose UAX #14 class is a mandatory break, so the break iterator honours it with no
    /// special case in layout. A newline would break the same way and would read as the end of a
    /// paragraph to anything scanning the text later, which a break inside one is not.
    /// </remarks>
    private const char LineSeparator = '\u2028';

    /// <summary>Reads a <c>p:txBody</c> or <c>a:txBody</c>.</summary>
    /// <param name="body">The text body element.</param>
    /// <param name="theme">The theme, for themed run colours.</param>
    /// <param name="defaultTypeface">The typeface a run that names none falls back to.</param>
    /// <param name="inherited">
    /// The per-level property sources outside the body, most specific first — normally
    /// <see cref="PptxTextStyles.LevelPropertiesFor"/>. Null reads the body alone, which is right
    /// for a shape with no placeholder chain behind it and wrong for every slide placeholder.
    /// </param>
    /// <param name="fields">
    /// What this slide's automatic fields resolve to, or null to draw the cached text instead.
    /// </param>
    /// <param name="inheritedBodyProperties">
    /// <para>
    /// The <c>a:bodyPr</c> of each placeholder behind this shape, nearest first — normally
    /// <see cref="PptxTextStyles.BodyPropertiesFor"/>.
    /// </para>
    /// <para>
    /// <strong>A placeholder's body properties are inherited, attribute by attribute.</strong>
    /// <c>PPTShapeContext</c> copy-constructs the slide shape's text body from the one
    /// <c>applyShapeReference</c> already brought over from the layout or master placeholder
    /// (<c>oox/source/ppt/pptshapecontext.cxx:183-186</c>), so the slide's own
    /// <c>&lt;a:bodyPr/&gt;</c> — which is what PowerPoint writes on a placeholder it has not
    /// re-formatted — overrides nothing, and the anchor, the insets, the wrap and the autofit all
    /// come from above.
    /// </para>
    /// <para>
    /// It matters most for the anchor. <c>chapter_4_0.pptx</c> states <c>anchor="ctr"</c> once,
    /// on the master's footer, and its footer runs to two lines in a one-line box on all 55
    /// slides: centred, both lines are on the page; anchored to the top, the second falls off the
    /// bottom edge and three words a page go with it.
    /// </para>
    /// </param>
    /// <param name="shapeTextStyle">
    /// <para>
    /// What the shape's own <c>p:style/a:fontRef</c> states — normally
    /// <see cref="PptxTextStyles.ShapeTextStyleFor"/>. It is the one rung of the chain with no
    /// element inside the text body, so it cannot arrive as part of
    /// <paramref name="inherited"/>: the colour sits directly under the <c>a:fontRef</c> and the
    /// typefaces are an <c>idx</c> into the theme's font scheme.
    /// </para>
    /// <para>
    /// Null leaves the chain as the body and its placeholders describe it, which is right for a
    /// shape with no <c>p:style</c> and wrong for the 1197 corpus slide shapes that have one.
    /// </para>
    /// </param>
    public static SlideTextBody Read(
        XElement body,
        DrawingTheme? theme = null,
        string? defaultTypeface = null,
        Func<int, IReadOnlyList<XElement>>? inherited = null,
        SlideFields? fields = null,
        IReadOnlyList<XElement?>? inheritedBodyProperties = null,
        DrawingCharacterStyle? shapeTextStyle = null)
    {
        ArgumentNullException.ThrowIfNull(body);

        List<XElement> bodyChain = [];
        if (Drawing.Child(body, "bodyPr") is { } own) bodyChain.Add(own);
        if (inheritedBodyProperties is not null)
        {
            foreach (XElement? source in inheritedBodyProperties)
            {
                if (source is not null) bodyChain.Add(source);
            }
        }

        XElement? properties = bodyChain.Count > 0 ? bodyChain[0] : null;
        XElement? listStyle = Drawing.Child(body, "lstStyle");

        // One counter and one "is this level numbering" flag per outline level, carried across
        // the whole body: a:buAutoNum numbers a *run* of paragraphs, and the run is broken by a
        // paragraph that draws a different kind of marker or none at all.
        int[] counters = new int[9];
        bool[] counting = new bool[9];

        // And one bullet face per level, for the same reason and by the same mechanism: the
        // shape's numbering rule outlives the paragraph that wrote to it. See `Marker`.
        string?[] markerFaces = new string?[9];

        List<SlideParagraph> paragraphs = [];
        foreach (XElement paragraph in Drawing.Children(body, "p"))
        {
            paragraphs.Add(
                Paragraph(
                    paragraph, listStyle, theme, defaultTypeface, counters, counting,
                    markerFaces, inherited, fields, shapeTextStyle));
        }

        // The autofit choice is taken whole from the nearest a:bodyPr that states one of the
        // three: a slide's <a:bodyPr/> saying nothing is not the same as its saying a:noAutofit.
        //
        // `resizes` records *which* of the three that nearest one was, which @wrap needs and
        // @fontScale does not — see Wraps below. Where one element states more than one, which is
        // malformed, normAutofit wins over spAutoFit and both over noAutofit; the order of the
        // tests is the precedence.
        XElement? autofit = null;
        bool resizes = false;
        foreach (XElement source in bodyChain)
        {
            if (Drawing.Child(source, "normAutofit") is { } stated)
            {
                autofit = stated;
                resizes = true;
                break;
            }

            if (Drawing.Child(source, "spAutoFit") is not null) { resizes = true; break; }
            if (Drawing.Child(source, "noAutofit") is not null) break;
        }

        (string? Preset, IReadOnlyList<FontworkAdjustment> Adjustments) warp = Warp(bodyChain);

        return new SlideTextBody
        {
            Paragraphs = paragraphs,
            Insets = Insets(bodyChain),
            Anchor = Anchor(Stated(bodyChain, "anchor")),
            Rotation = Rotation(bodyChain),
            // wrap="none" does not stand on its own: it suppresses wrapping only while the
            // body's autofit leaves the shape alone. Measured on 26.2.4.2 with nine authored
            // one-shape variants over both axes, a 236 pt box holding a 64-character line —
            // `wrap="none"` draws one line with a:noAutofit and with no autofit element at all,
            // and **four** lines with a:spAutoFit or a:normAutofit; `wrap="square"` draws four
            // in all four autofit cases. So a fitting autofit beats the wrap, which is the one
            // combination this read as unbounded.
            //
            // It is not a cosmetic difference. Treating a wrap="none" + spAutoFit body as
            // unbounded runs its line off the *page*, not merely off the shape, and everything
            // past the media box is lost from the text layer: 30 of the 305 baseline renderings
            // drew text outside the page against the reference's 9, and one template family
            // overhangs 720 pt by 8.7 pt so that `Google Slides` extracts as `Google Slid`.
            // That two-character loss is the whole character difference on 15 of the 28
            // documents filed as `text`.
            Wraps = Stated(bodyChain, "wrap") != "none" || resizes,
            AutoFit = autofit is not null,
            FontScale = Thousandth(autofit, "fontScale", 1.0),
            WarpPreset = warp.Preset,
            WarpAdjustments = warp.Adjustments,

            // a:normAutofit/@lnSpcReduction is deliberately not read: neither does the reference,
            // whose normAutofit handler takes @fontScale alone. See SlideTextBody.AutoFit.
        };
    }

    /// <summary>
    /// The four text insets, each defaulting to DrawingML's own default rather than to zero.
    /// </summary>
    /// <remarks>
    /// 91440 EMU across and 45720 down — a tenth and a twentieth of an inch. A body that states
    /// none gets exactly those, so defaulting them to zero moves every line of every unstated
    /// text box 7.2 pt left and 3.6 pt up. Measured on <c>shape-geometry.pptx</c> slide 3, whose
    /// second box states them explicitly and whose first states zero: LibreOffice draws the two
    /// pens 7.2 pt apart.
    /// </remarks>
    private static Margins Insets(List<XElement> chain)
    {
        Length left = Length.FromEmu(Emu(Stated(chain, "lIns"), 91440));
        Length top = Length.FromEmu(Emu(Stated(chain, "tIns"), 45720));
        Length right = Length.FromEmu(Emu(Stated(chain, "rIns"), 91440));
        Length bottom = Length.FromEmu(Emu(Stated(chain, "bIns"), 45720));

        // A vertical body's insets travel with its text, and they are stated against the shape
        // rather than against the turned frame. `TextBodyProperties::pushTextDistances`
        // (`oox/source/drawingml/textbodyproperties.cxx:68-129`) walks the four in order and
        // writes them into slots starting at 3 for vert/eaVert and at 1 for vert270 — a cyclic
        // shift, and the same one in the same direction as the turn.
        //
        // The layout here happens in the TRANSPOSED rectangle (see PptxSlideLayout.Turned), so
        // what this has to produce is the quadruple that frame wants. Measured rather than
        // reasoned: an authored box with lIns/tIns/rIns/bIns of 10/20/30/40 pt, rendered by
        // 26.2.4.2, puts the first vert baseline 30 pt in from the transposed frame's top and
        // 20 pt in from its left, and the vert270 one 10 pt and 40 pt — so
        //
        //     vert, eaVert:  (left, top, right, bottom) <- (tIns, rIns, bIns, lIns)
        //     vert270:       (left, top, right, bottom) <- (bIns, lIns, tIns, rIns)
        //
        // This is not neutral on a body that states nothing: DrawingML's defaults are 0.1 inch
        // across and 0.05 inch down, so a turned body gets 0.05 inch left and right and 0.1 inch
        // top and bottom. Leaving the quadruple alone would be wrong by 3.6 pt on every edge of
        // every vertical shape.
        return Vertical(chain) switch
        {
            VerticalText.Clockwise => new Margins(top, right, bottom, left),
            VerticalText.Anticlockwise => new Margins(bottom, left, top, right),
            _ => new Margins(left, top, right, bottom),
        };
    }

    /// <summary>Which quarter turn <c>a:bodyPr/@vert</c> asks for, if any.</summary>
    private enum VerticalText
    {
        /// <summary>The body reads across, which is everything but the three values below.</summary>
        None,

        /// <summary><c>vert</c> and <c>eaVert</c>: the text reads downwards.</summary>
        Clockwise,

        /// <summary><c>vert270</c>: the text reads upwards.</summary>
        Anticlockwise,
    }

    /// <summary>
    /// Reads <c>a:bodyPr/@vert</c> as a quarter turn.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>eaVert</c> and <c>vert</c> are <b>the same rendering</b>, and that is measured rather
    /// than assumed: the importer sends them down different property paths —
    /// <c>WritingMode2::TB_RL</c> against <c>TB_RL90</c>, with a different horizontal and vertical
    /// adjust apiece (<c>oox/source/drawingml/textbodypropertiescontext.cxx:126-200</c>) — and an
    /// authored deck holding everything else fixed over the three anchors draws
    /// <b>165 identical glyph matrices at identical positions</b> for the two. Latin text in an
    /// East-Asian vertical frame is simply turned.
    /// </para>
    /// <para>
    /// The two values the corpus does not use are deliberately <see cref="VerticalText.None"/>
    /// and were measured too, on the same deck: <c>mongolianVert</c> draws <b>horizontally</b>
    /// — the importer's own comment says the rendering is not implemented for shape text — and
    /// <c>wordArtVert</c> draws one glyph per line, upright and stacked
    /// (<c>WritingMode2::STACKED</c>), which is not a turn and would be wrong to model as one.
    /// Over the 302-document slides corpus neither appears on any slide part.
    /// </para>
    /// </remarks>
    private static VerticalText Vertical(List<XElement> chain) => Stated(chain, "vert") switch
    {
        "vert" or "eaVert" => VerticalText.Clockwise,
        "vert270" => VerticalText.Anticlockwise,
        _ => VerticalText.None,
    };

    /// <summary>
    /// The turn <c>a:bodyPr/@rot</c> asks for, in radians clockwise.
    /// </summary>
    /// <remarks>
    /// Sixtieth-thousandths of a degree, clockwise, like every other DrawingML angle — and unlike
    /// ODF's, which runs the other way. The attribute is what a SmartArt <c>autoTxRot</c>
    /// resolves to, so the diagram evaluator writes it and this reads it back through the same
    /// path an authored deck's would take.
    /// </remarks>
    private static double Rotation(List<XElement> chain)
    {
        int units = int.TryParse(
            Stated(chain, "rot"), NumberStyles.Integer, CultureInfo.InvariantCulture,
            out int stated)
            ? stated
            : 0;

        // @vert is the other half of the same quantity for everything downstream: a body that
        // reads downwards is a body whose frame is transposed and whose runs travel with a
        // quarter-turn matrix, which is exactly what a quarter of @rot already produces. Adding
        // rather than replacing is what the reference does with the two — `a:bodyPr` can state
        // both, and `Transform2DContext` accumulates into the same field.
        //
        // Measured against 26.2.4.2 on an authored deck: a 230 x 160 pt box at (15.748, 23.622)
        // holding 14 pt text draws its first vert glyph at (231.732, 516.416) and its first
        // vert270 glyph at (29.735, 356.428); transposing about the centre and turning by a
        // quarter reproduces both to 0.03 pt, and the line pitch of 16.809 stacks leftwards for
        // vert and rightwards for vert270 exactly as the reference stacks it.
        units += Vertical(chain) switch
        {
            VerticalText.Clockwise => 90 * (int)ShapeTransform.RotationUnitsPerDegree,
            VerticalText.Anticlockwise => -90 * (int)ShapeTransform.RotationUnitsPerDegree,
            _ => 0,
        };

        return units == 0
            ? 0
            : units / ShapeTransform.RotationUnitsPerDegree * Math.PI / 180.0;
    }

    /// <summary>
    /// The WordArt preset <c>a:bodyPr/a:prstTxWarp</c> asks for, or null when the body is
    /// ordinary text.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>textNoShape</c> is normalised away to null because it is the value that means
    /// <em>no</em> warp, and it is overwhelmingly the common one: of the 67 documents in the
    /// corpus's slides track that carry a <c>prstTxWarp</c> at all, 65 carry only
    /// <c>textNoShape</c>. Reading the element's presence rather than its value would turn a
    /// two-document behaviour into a sixty-seven-document one.
    /// </para>
    /// <para>
    /// The chain is walked in the same most-specific-first order the rest of the body
    /// properties use, because <c>PPTShapeContext</c> copy-constructs the slide shape's text
    /// body from the placeholder's, so a warp stated on a layout or master placeholder is
    /// inherited exactly as an anchor or an inset is.
    /// </para>
    /// </remarks>
    private static (string? Preset, IReadOnlyList<FontworkAdjustment> Adjustments) Warp(
        List<XElement> chain)
    {
        foreach (XElement source in chain)
        {
            if (Drawing.Child(source, "prstTxWarp") is not { } warp) continue;

            string? preset = warp.Attribute("prst")?.Value;
            if (string.IsNullOrEmpty(preset) || preset == "textNoShape") return (null, []);

            List<FontworkAdjustment> adjustments = [];
            foreach (XElement guide in Drawing.Children(Drawing.Child(warp, "avLst"), "gd"))
            {
                string name = guide.Attribute("name")?.Value ?? string.Empty;
                string formula = guide.Attribute("fmla")?.Value ?? string.Empty;

                // Every guide inside a `prstTxWarp` is a literal: `fmla="val 10800000"`.
                if (!formula.StartsWith("val ", StringComparison.Ordinal)) continue;
                if (!double.TryParse(
                        formula.AsSpan(4), NumberStyles.Float, CultureInfo.InvariantCulture,
                        out double value))
                {
                    continue;
                }

                adjustments.Add(new FontworkAdjustment(name, value));
            }

            return (preset, adjustments);
        }

        return (null, []);
    }

    /// <summary>
    /// What an <c>a:fld</c> of this type draws, or null to fall back to its cached text.
    /// </summary>
    /// <remarks>
    /// Only the two that are a property of the deck rather than of the machine reading it.
    /// A date or a file name is deliberately left as cached: the reference substitutes the
    /// conversion's own clock and path, and reproducing that would make a rendering
    /// unreproducible.
    /// </remarks>
    private static string? FieldText(string? type, SlideFields? fields)
    {
        if (fields is not { } known) return null;

        return type switch
        {
            "slidenum" => known.Number.ToString(CultureInfo.InvariantCulture),
            "slidecount" => known.Count.ToString(CultureInfo.InvariantCulture),
            _ => null,
        };
    }

    private static TextAnchor Anchor(string? anchor) => anchor switch
    {
        "ctr" => TextAnchor.Middle,
        "b" => TextAnchor.Bottom,
        _ => TextAnchor.Top,
    };

    private static SlideParagraph Paragraph(
        XElement paragraph,
        XElement? listStyle,
        DrawingTheme? theme,
        string? defaultTypeface,
        int[] counters,
        bool[] counting,
        string?[] markerFaces,
        Func<int, IReadOnlyList<XElement>>? inherited,
        SlideFields? fields,
        DrawingCharacterStyle? shapeTextStyle)
    {
        XElement? paragraphProperties = Drawing.Child(paragraph, "pPr");
        int level = Math.Clamp(Drawing.Number(paragraphProperties, "lvl") ?? 0, 0, 8);

        // The paragraph's own properties, then the body's own list style for this level, then
        // everything outside the body. Materialised because it is walked once per property.
        List<XElement> chain = [];
        if (paragraphProperties is not null) chain.Add(paragraphProperties);
        if (LevelStyle(listStyle, level) is { } levelStyle) chain.Add(levelStyle);
        int withinBody = chain.Count;
        if (inherited is not null) chain.AddRange(inherited(level));

        // Every a:defRPr the chain offers, most specific first. A run states only what differs
        // from these, and on a PowerPoint-authored deck it commonly states nothing at all.
        XElement?[] defaults =
            [.. chain.Select(source => Drawing.Child(source, "defRPr")).OfType<XElement>()];

        // Where the shape's own p:style/a:fontRef sits, and it is neither end: it beats
        // everything the shape inherits and loses to everything the body states. Counting the
        // *sources* rather than the surviving a:defRPr elements would put the boundary in the
        // wrong place whenever a paragraph or a level style has no defRPr at all, which is the
        // common case.
        int bodyDefaults = chain
            .Take(withinBody)
            .Count(source => Drawing.Child(source, "defRPr") is not null);

        RunSources sources = new(defaults, bodyDefaults, shapeTextStyle);

        StringBuilder text = new();
        List<SlideTextRun> runs = [];

        foreach (XElement child in paragraph.Elements())
        {
            if (Drawing.Is(child, "r"))
            {
                string content = Drawing.Child(child, "t")?.Value ?? string.Empty;
                if (content.Length == 0) continue;

                runs.Add(Run(
                    Drawing.Child(child, "rPr"), sources, text.Length, content.Length,
                    theme, defaultTypeface));
                text.Append(Capitalised(content, Drawing.Child(child, "rPr"), sources));
            }
            else if (Drawing.Is(child, "br"))
            {
                text.Append(LineSeparator);
            }
            else if (Drawing.Is(child, "fld"))
            {
                // The slide's own position, and otherwise the cached value. A field is not a run
                // with stale text: TextField::insertAt turns a slidenum into a
                // com.sun.star.text.TextField.PageNumber (textfield.cxx:107-111), which draws the
                // page it lands on. It matters because the cached text of the field on a *master*
                // is the literal placeholder "‹#›" — one master shape serving forty slides cannot
                // cache forty different numbers — so drawing the cache puts "‹#›" on every page of
                // any deck whose page number lives on the master rather than on each slide.
                string content =
                    FieldText(Drawing.Attribute(child, "type"), fields)
                    ?? Drawing.Child(child, "t")?.Value
                    ?? string.Empty;
                if (content.Length == 0) continue;

                runs.Add(Run(
                    Drawing.Child(child, "rPr"), sources, text.Length, content.Length,
                    theme, defaultTypeface));
                text.Append(Capitalised(content, Drawing.Child(child, "rPr"), sources));
            }
        }

        if (runs.Count == 0)
        {
            // An empty paragraph is still a line, and it is as tall as the text that would go on
            // it — which is what a:endParaRPr records and the only thing it is for.
            runs.Add(Run(
                Drawing.Child(paragraph, "endParaRPr"), sources, 0, 0, theme, defaultTypeface));
        }

        // The size a percentage spacing is a percentage of: the tallest run in the paragraph, as
        // LibreOffice takes it (textparagraph.cxx:131, `nCharHeight = std::max(...)`).
        Length tallest = Length.Zero;
        foreach (SlideTextRun run in runs)
        {
            if (run.Size > tallest) tallest = run.Size;
        }

        return new SlideParagraph(
            text.ToString(),
            runs,
            Alignment(Stated(chain, "algn")),
            Spacing(Child(chain, "spcBef"), tallest),
            Spacing(Child(chain, "spcAft"), tallest),
            LineSpacing(Child(chain, "lnSpc")),
            Length.FromEmu(Emu(chain, "marL")),
            Length.FromEmu(Emu(chain, "indent")),
            Language(Drawing.Child(paragraph, "r")),
            Marker(chain, theme, level, counters, counting, markerFaces, text.Length > 0))
        {
            // a:defTabSz, whose absence means DrawingML's own default of one inch and not a word
            // processor's half inch. Nearly every master states it explicitly as 914400, which is
            // the same inch — so the value that matters is the fallback.
            DefaultTabInterval = Stated(chain, "defTabSz") is { } stated
                                 && long.TryParse(
                                     stated, NumberStyles.Integer, CultureInfo.InvariantCulture,
                                     out long emu)
                                 && emu > 0
                ? Length.FromEmu(emu)
                : SlideParagraph.DefaultTabDistance,
        };
    }

    /// <summary>The first source in the chain to state an attribute.</summary>
    private static string? Stated(List<XElement> chain, string attribute)
    {
        foreach (XElement source in chain)
        {
            if (Drawing.Attribute(source, attribute) is { } value) return value;
        }
        return null;
    }

    /// <summary>The first source in the chain to carry a child element.</summary>
    private static XElement? Child(List<XElement> chain, string name)
    {
        foreach (XElement source in chain)
        {
            if (Drawing.Child(source, name) is { } child) return child;
        }
        return null;
    }

    /// <summary>
    /// The bullet a paragraph draws, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The bullet elements are a choice: <c>a:buNone</c>, <c>a:buChar</c> or <c>a:buAutoNum</c>,
    /// and the first source in the chain stating any of them settles it — including
    /// <c>a:buNone</c>, which is the point of writing one. Continuing past a <c>buNone</c> to a
    /// source that does state a bullet would put bullets back on every title, because a master's
    /// title placeholder is exactly a <c>buNone</c> over a body style that bullets.
    /// </para>
    /// <para>
    /// A Private Use Area character is substituted for U+2022 the way extraction already does.
    /// Those code points are Wingdings and Symbol positions and mean nothing outside those fonts,
    /// which are not installed on the machines this runs on.
    /// </para>
    /// <para>
    /// <c>a:buAutoNum</c> is numbered by the counters the caller carries across the body, which
    /// is the same walk and the same arrays extraction uses — <see cref="DrawingTextBody"/> owns
    /// the arithmetic so that a nested list cannot be numbered two different ways by the two
    /// readers. <strong>An empty paragraph draws no marker and consumes no number:</strong> the
    /// blank line an author leaves between two items is still an <c>a:p</c> and still inherits
    /// the level's bullet, and counting it makes the next item jump from 2 to 4.
    /// </para>
    /// <para>
    /// <strong>A stated bullet face outlives the paragraph that states it, per level.</strong>
    /// Each paragraph writes its bullet properties into the <em>shape's</em> numbering rule —
    /// <c>xNumRule-&gt;replaceByIndex(getLevel(), …)</c>,
    /// <c>oox/source/drawingml/textparagraph.cxx:191</c> over
    /// <c>textparagraphproperties.cxx:483-502</c> — and
    /// <c>SvxUnoNumberingRules::replaceByIndex</c> starts from the level's <em>existing</em>
    /// format (<c>editeng/source/uno/unonrule.cxx:257</c>). A paragraph that pushes no
    /// <c>PROP_BulletFont</c>, because it names none or because it says <c>a:buFontTx</c>,
    /// therefore keeps whatever the last paragraph at its level left there. The bullet map itself
    /// is per paragraph (<c>textparagraph.cxx:136</c>) and carries nothing; the rule is what
    /// persists.
    /// </para>
    /// <para>
    /// Measured through <c>soffice</c> 26.2.4.2 on a four-paragraph probe body: a first paragraph
    /// stating <c>a:buFont typeface="Courier New"</c> and <c>a:buNone</c> — so drawing no bullet
    /// at all — makes the next level-0 paragraph's faceless <c>a:buChar</c> draw from
    /// <b>LiberationMono</b>, while the level-1 paragraph between them draws from
    /// <b>OpenSymbol</b>, and a following <c>a:buFontTx</c> paragraph draws from
    /// <b>LiberationMono</b> again. So it is per level, it survives <c>buNone</c>, and it is what
    /// <c>a:buFontTx</c> actually resolves to whenever a sibling has named a face.
    /// </para>
    /// </remarks>
    private static SlideMarker? Marker(
        List<XElement> chain,
        DrawingTheme? theme,
        int level,
        int[] counters,
        bool[] counting,
        string?[] markerFaces,
        bool hasText)
    {
        int slot = Math.Clamp(level, 0, counters.Length - 1);

        // Before any of the branches below, and whether or not this paragraph draws anything:
        // stating a face writes it into the level's rule, and a `buNone` paragraph states it just
        // as loudly as a bulleted one.
        if (!FollowsTextFont(chain)
            && Drawing.Attribute(Child(chain, "buFont"), "typeface") is { Length: > 0 } stated)
        {
            markerFaces[slot] = stated;
        }

        foreach (XElement source in chain)
        {
            if (Drawing.Child(source, "buNone") is not null)
            {
                counting[slot] = false;
                return null;
            }

            if (Drawing.Child(source, "buAutoNum") is { } number)
            {
                if (!hasText) return null;

                return Marked(
                    DrawingTextBody.AutoNumber(number, slot, counters, counting),
                    chain, theme, markerFaces[slot], isSymbol: false);
            }

            if (Drawing.Child(source, "buChar") is not { } bullet) continue;

            counting[slot] = false;
            if (!hasText) return null;

            string? character = Drawing.Attribute(bullet, "char");
            if (string.IsNullOrEmpty(character)) return null;

            // The Private Use Area collapse belongs to `Marked`, which is the only place that
            // knows whether the face has a recode table. Doing it here destroyed the slot before
            // the table could be reached: a bullet stated as `char="&#xF0D8;"` — which is a
            // quarter of the corpus's symbol bullets — became U+2022 and was then re-symbolised
            // into slot 0x22, so every one of them drew the *same* wrong glyph.
            return Marked(FirstCodePoint(character), chain, theme, markerFaces[slot]);
        }

        counting[slot] = false;
        return null;
    }

    /// <summary>
    /// The first code point of a bullet character, which is all of it a bullet may be.
    /// </summary>
    /// <remarks>
    /// <c>a:buChar/@char</c> is an <c>ST_Char</c>: one character, and real files break that.
    /// <c>sd/qa/unit/data/pptx/bnc862510_5.pptx</c> writes
    /// <c>&lt;a:buChar char="••"/&gt;</c> in a SmartArt shape, and drawing what it says puts a
    /// second bullet where the reference draws the text's first letter — 22.5 pt of overlap on a
    /// 40 pt line, because the hanging indent goes to <c>marL</c> whatever the marker's width
    /// turned out to be. LibreOffice keeps the whole string through its import
    /// (<c>textparagraphproperties.cxx:326</c>) and truncates where the numbering rule is built:
    /// <c>aFmt.SetBulletChar(aStr.iterateCodePoints(…))</c>,
    /// <c>editeng/source/uno/unonrule.cxx:320</c>. A code point rather than a UTF-16 unit, so an
    /// astral bullet survives.
    /// </remarks>
    private static string FirstCodePoint(string character)
        => char.IsHighSurrogate(character[0]) && character.Length > 1
            ? character[..2]
            : character[..1];

    /// <summary>
    /// A marker's text with the font, size and colour the chain gives it.
    /// </summary>
    /// <remarks>
    /// Each satellite property is looked up down the whole chain separately from the bullet
    /// character, because a paragraph routinely states the character and leaves the font, size
    /// and colour to its level — and because the three are each their own element rather than
    /// attributes of the bullet. The face is the exception and arrives as
    /// <paramref name="typeface"/>: it belongs to the level rather than to the paragraph, so
    /// <see cref="Marker"/> resolves it. Null means "nothing has named one", which
    /// <c>SlideTextLayout</c> draws from OpenSymbol.
    /// </remarks>
    private static SlideMarker Marked(
        string text,
        List<XElement> chain,
        DrawingTheme? theme,
        string? typeface,
        bool isSymbol = true)
    {
        // Whether *this* paragraph's own chain makes it a symbol position, which is the question
        // `pushToPropMap` asks when it decides whether to move the character into the Private Use
        // Area — a decision it takes per paragraph, unlike the face, which is the level's.
        XElement? font = FollowsTextFont(chain) ? null : Child(chain, "buFont");

        // Only a stated character is a symbol position. A generated number is digits whatever
        // face the level names for its bullet, and recoding it would make nonsense of it.
        bool symbolFont = isSymbol && IsSymbolFont(font);

        // A recodeable face keeps its Private Use Area slot: SlideTextLayout turns it into the
        // OpenSymbol glyph holding the same picture, which needs the slot and the face together.
        // Everything else is collapsed to U+2022 here, which is what the whole of this method's
        // input used to arrive already collapsed to.
        bool recodeable = symbolFont && SymbolFontRecode.IsRecodeable(typeface);
        string symbolised = symbolFont ? Symbolised(text) : text;

        return new SlideMarker(
                recodeable ? symbolised : OutlineNumbers.NormaliseBullet(symbolised),

                // The face is kept even for a symbol bullet: LibreOffice sets both PROP_BulletFont
                // and PROP_BulletFontName from it and lets the substitution find OpenSymbol
                // (textparagraphproperties.cxx:347-348). Dropping it here draws the bullet in the
                // body face instead, which is a different glyph and a different embedded program.
                typeface,
                Drawing.Number(Child(chain, "buSzPct"), "val") is { } percent && percent > 0
                    ? percent / 100000.0
                    : 1.0,
                ColourIn(Child(chain, "buClr"), theme),
                isSymbol);
    }

    /// <summary>
    /// Whether the chain asks for the bullet to be set in the text's own face — which, in
    /// LibreOffice, means it is set in <em>neither</em> the text's face nor an <c>a:buFont</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>a:buFontTx</c> sets <c>mbBulletFontFollowText</c>, and the import's whole response to it
    /// is to <em>skip</em> the block that would set <c>PROP_BulletFont</c> — nothing else is put in
    /// its place (<c>oox/source/drawingml/textparagraphproperties.cxx:311-347</c>). So the bullet
    /// keeps the numbering rule's default face, which for Impress is OpenSymbol, and the element
    /// that reads as "follow the text" in fact means "state no face at all". Measured: a paragraph
    /// with <c>a:buFontTx</c> over text in Courier New draws its bullet from OpenSymbol, not from
    /// LiberationMono.
    /// </para>
    /// <para>
    /// It is looked for anywhere in the chain rather than at its most specific rung, because
    /// <c>BulletList::apply</c> only ever assigns the flag when the source <em>has</em> a value and
    /// nothing writes a false one (<c>textparagraphproperties.cxx:267-268</c>, with the only
    /// producer at <c>textparagraphpropertiescontext.cxx:282-283</c>). A master that says
    /// <c>buFontTx</c> therefore outlives a paragraph that names a face.
    /// </para>
    /// <para>
    /// It suppresses only <em>this</em> paragraph's push. What the level's rule already holds from
    /// an earlier sibling stands — see <see cref="Marker"/>, where that is carried.
    /// </para>
    /// </remarks>
    private static bool FollowsTextFont(List<XElement> chain)
    {
        foreach (XElement source in chain)
        {
            if (Drawing.Child(source, "buFontTx") is not null) return true;
        }
        return false;
    }

    /// <summary>
    /// Whether an <c>a:buFont</c> names a face whose code points are symbol positions rather than
    /// characters.
    /// </summary>
    /// <remarks>
    /// <c>charset="2"</c> is <c>SYMBOL_CHARSET</c> and is the attribute that decides it; the name
    /// list behind it is LibreOffice's, which checks both because writers omit the charset on the
    /// very faces that most need it (<c>oox/source/drawingml/textparagraphproperties.cxx:334-346</c>,
    /// with the charset read at <c>textfont.cxx:92</c>).
    /// </remarks>
    private static bool IsSymbolFont(XElement? font)
    {
        if (font is null) return false;
        if (Drawing.Attribute(font, "charset") is "2" or "0x02") return true;

        string? typeface = Drawing.Attribute(font, "typeface");
        return typeface is not null && SymbolFaces.Contains(typeface);
    }

    private static readonly HashSet<string> SymbolFaces = new(StringComparer.OrdinalIgnoreCase)
    {
        "Wingdings", "Wingdings 2", "Wingdings 3", "Monotype Sorts", "Monotype Sorts 2",
        "Webdings", "StarBats", "StarMath", "ZapfDingbats",
    };

    /// <summary>
    /// Moves a symbol-font bullet into the Private Use Area the way the reference does.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A file states a Wingdings bullet by the byte its position has in the legacy symbol
    /// encoding, so <c>char="§"</c> means position 0xA7 and not the section sign. LibreOffice
    /// rewrites it as <c>(c &amp; 0x00ff) | 0xf000</c>
    /// (<c>oox/source/drawingml/textparagraphproperties.cxx:358-361</c>), which is where the
    /// glyph really lives in the symbol-encoded face.
    /// </para>
    /// <para>
    /// Doing only that would draw nothing on a machine without the face, so the result goes on
    /// through <see cref="OutlineNumbers.NormaliseBullet"/> to U+2022 — the same two steps the
    /// binary path already takes (<c>PptTextReader.Symbolised</c>), and the reason a
    /// <c>.ppt</c> and a <c>.pptx</c> of the same deck no longer draw different bullets.
    /// </para>
    /// </remarks>
    private static string Symbolised(string bullet)
        => bullet.Length == 1 ? ((char)(0xF000 | (bullet[0] & 0x00FF))).ToString() : bullet;

    /// <summary>
    /// The colour a wrapper element holds directly, rather than through an <c>a:solidFill</c>.
    /// </summary>
    /// <remarks>
    /// <c>a:buClr</c> is one: it holds the colour reference outright, where an <c>a:rPr</c> wraps
    /// it in a fill. Two readers rather than one because the two shapes really do differ, and a
    /// reader looking for a fill inside a <c>buClr</c> finds nothing and draws a black bullet on a
    /// deck that asked for a coloured one.
    /// </remarks>
    private static Colour? ColourIn(XElement? wrapper, DrawingTheme? theme)
    {
        if (wrapper is null) return null;

        foreach (XElement child in wrapper.Elements())
        {
            if (DrawingColour.Read(child)?.Resolve(theme) is { } colour) return colour;
        }

        return null;
    }

    private static string? Language(XElement? run)
        => Drawing.Attribute(Drawing.Child(run, "rPr"), "lang");

    /// <summary>
    /// A typeface attribute with no theme to follow it: the name itself, or nothing when it is an
    /// unfollowable reference.
    /// </summary>
    /// <remarks>
    /// Reporting <c>+mn-lt</c> as a family name is worse than reporting none, because one falls
    /// back to the next source in the chain and the other ends the search on a face that does not
    /// exist.
    /// </remarks>
    private static string? Literal(string? typeface)
        => string.IsNullOrEmpty(typeface) || typeface[0] == '+' ? null : typeface;

    private static SlideTextRun Run(
        XElement? runProperties,
        RunSources sources,
        int start,
        int length,
        DrawingTheme? theme,
        string? defaultTypeface)
    {
        XElement?[] defaults = sources.Defaults;

        int size = First(runProperties, defaults, element => Drawing.Number(element, "sz"))
                   ?? DefaultSizeHundredthsOfPoint;

        bool bold = First(runProperties, defaults, element => Drawing.Flag(element, "b")) ?? false;
        bool italic = First(runProperties, defaults, element => Drawing.Flag(element, "i")) ?? false;

        // A theme reference rather than a family name is the normal case here, not the exception:
        // a master's txStyles states <a:latin typeface="+mn-lt"/> and every body placeholder in the
        // deck inherits it. Taking the attribute at face value ends the font search on a family
        // called "+mn-lt", which exists nowhere and falls all the way through to the generic
        // sans-serif — DejaVu Sans against the reference's Carlito, some 39 per cent wider, so
        // every line breaks early and the tail of a full placeholder overflows off the slide.
        string? typeface = sources.Resolve(
            runProperties,
            style => style.LatinTypeface,
            element => theme?.Fonts is { } fonts
                ? fonts.Resolve(Drawing.Attribute(Drawing.Child(element, "latin"), "typeface"))
                : Literal(Drawing.Attribute(Drawing.Child(element, "latin"), "typeface")));

        // a:rPr/a:sym — the face the run's private-use characters are drawn from, and it inherits
        // down the same chain everything else here does: TextCharacterProperties::assignUsed takes
        // maSymbolFont from the source whenever the source states one
        // (oox/source/drawingml/textcharacterproperties.cxx:55), so a level's a:defRPr can carry
        // it for every run under it. Theme-resolved for the same reason a:latin is: getFontData
        // puts the name through Theme::resolveFont before using it (textfont.cxx:80-85), so a
        // "+mn-lt" here is the minor latin face and not a family of that name.
        //
        // The element is taken whole rather than attribute by attribute, because the typeface and
        // the charset only mean anything together — see SlideSymbolFont. `assignIfUsed` copies the
        // TextFont as a unit for exactly the same reason.
        XElement? symbol = First(
            runProperties, defaults, element => Drawing.Child(element, "sym"));

        SlideSymbolFont? symbolFont = null;
        if (symbol is not null)
        {
            string? family = theme?.Fonts is { } symbolFonts
                ? symbolFonts.Resolve(Drawing.Attribute(symbol, "typeface"))
                : Literal(Drawing.Attribute(symbol, "typeface"));

            // getFontData returns false for an empty name and no switch happens at all
            // (textfont.cxx:87-94), so a nameless a:sym is not an a:sym.
            if (!string.IsNullOrEmpty(family))
            {
                symbolFont = new SlideSymbolFont(
                    family,
                    Drawing.Number(symbol, "charset") == WindowsCharsetSymbol);
            }
        }

        Colour? colour = sources.Resolve(
            runProperties, style => style.Colour, element => RunColour(element, theme));

        // A run's own a:hlinkClick makes the run a *field*, and redecorates it: see IsField,
        // HyperlinkColour and the underline below. The run's *own* a:rPr is what decides, not the
        // inherited chain, because LibreOffice tests maTextCharacterProperties — the run's
        // properties before the defaults are merged in — rather than the merged set
        // (oox/source/drawingml/textrun.cxx:88, :162-168).
        //
        // And the element is not the test: what textrun.cxx:88 asks is whether the run's
        // hyperlink property *map* is empty, which an a:hlinkClick stating nothing leaves it.
        // See DrawingHyperlink, and probes/pptx-field-r82 for the pair of variants that settle
        // it at the reference.
        XElement? hyperlink = Drawing.Child(runProperties, "hlinkClick");
        bool field = DrawingHyperlink.MakesField(hyperlink);
        if (field && !DrawingHyperlink.StatesItsOwnColour(hyperlink))
        {
            colour = HyperlinkColour(runProperties, sources, theme) ?? colour;
        }

        // a:rPr/@spc, in hundredths of a point, and negative far more often than not: a deck's
        // designer pulls a heading in and PowerPoint records it per run. LibreOffice reads it into
        // CharKerning (oox/source/drawingml/textcharacterproperties.cxx:190) and EditEngine adds it
        // between characters, so a run that states it is measurably narrower than the same text
        // without it — 10 pt over a 50-character line at the corpus's commonest value of -20.
        int tracking = First(runProperties, defaults, element => Drawing.Number(element, "spc")) ?? 0;

        // ST_TextUnderlineType, whose nineteen values are all "underlined" except "none" — the
        // dashes and the double rule differ in how the line is drawn and not in whether one is.
        // "sng" and "none" are all the corpus uses; anything else falls to a single rule rather
        // than to nothing, which is right far more often than the reverse.
        string? underline = First(
            runProperties, defaults, element => Drawing.Attribute(element, "u"));
        string? strike = First(
            runProperties, defaults, element => Drawing.Attribute(element, "strike"));

        // a:rPr/@baseline, in thousandths of a percent of the em, and the size that goes with it
        // is not in the file: LibreOffice divides the attribute by 1000 and pairs it with
        // DFLT_ESC_PROP, so 30000 means "raised 30% and set at 58%"
        // (oox/source/drawingml/textcharacterproperties.cxx:196-199). The shrink is the half that
        // moves line breaks — an ordinal set full size is 42% wider than the reference draws it.
        int baseline = First(runProperties, defaults, element => Drawing.Number(element, "baseline"))
                       ?? 0;


        return new SlideTextRun(
            start,
            length,
            typeface ?? defaultTypeface,
            Length.FromEmu(size * Length.EmuPerPoint / 100),
            bold ? 700 : 400,
            italic,
            colour ?? Colour.Black,
            Length.FromEmu(tracking * Length.EmuPerPoint / 100),
            // A hyperlink underlines itself unless the run states an underline of its own — and
            // "of its own" is again the run's a:rPr and not the chain behind it, so a defRPr
            // saying u="none" above a linked run does not stop the rule
            // (oox/source/drawingml/textrun.cxx:167-168).
            IsUnderlined: (field && Drawing.Attribute(runProperties, "u") is null)
                          || underline is not null and not "none",
            IsStruckThrough: strike is not null and not "noStrike",
            Escapement: baseline == 0
                ? SlideEscapement.None
                : new SlideEscapement(baseline / 1000, SlideEscapement.AutomaticProportion),
            SymbolFont: symbolFont,
            // A hyperlink run is an EditEngine field, which changes how it breaks and how the
            // lines it spills onto are stacked — see SlideTextRun.IsField and SlideTextLayout.
            IsField: field);
    }

    /// <summary>
    /// The first of the run's own properties and the defaults behind it that states something.
    /// </summary>
    /// <remarks>
    /// Attribute by attribute rather than element by element, which is what LibreOffice's
    /// <c>assignUsed</c> does (<c>oox/source/drawingml/textparagraph.cxx:51-67</c>): a run stating
    /// <c>b="1"</c> and nothing else has not cancelled the size its level's <c>defRPr</c> gives
    /// it. Merging whole property sets gives the right answer on every run that states everything
    /// — which is every run LibreOffice writes — and the wrong one everywhere it matters.
    /// </remarks>
    /// <summary>
    /// A run's text as <c>a:rPr/@cap</c> draws it.
    /// </summary>
    /// <remarks>
    /// <para>
    /// DrawingML's text capitalisation, which LibreOffice reads into <c>CharCaseMap</c>
    /// (<c>oox/source/drawingml/textcharacterpropertiescontext.cxx:79-80</c> takes the attribute,
    /// <c>textcharacterproperties.cxx:194</c> sets the property). It is inherited down the
    /// layout/master chain like any other run property, so it is resolved through
    /// <see cref="First{T}"/> rather than read off the run's own <c>a:rPr</c> — a deck that states
    /// it once on its master and nowhere else is the common case, and 71 of the corpus's decks
    /// state it at all.
    /// </para>
    /// <para>
    /// <strong>This is presentation, not content.</strong> It is applied here, on the layout
    /// path, and deliberately not in <c>PptxShapeReader</c>, which is what the extraction API
    /// reads: copying an all-caps-formatted run out of a deck yields the case the author typed.
    /// The reference's PDF holds the upper-case glyphs, which is why the rendering must match and
    /// the extraction must not.
    /// </para>
    /// <para>
    /// <strong>Only <c>all</c> is implemented.</strong> <c>small</c> needs real or synthesised
    /// small capitals — a smaller face for the lower-case letters, not merely upper-casing them —
    /// and LibreOffice's own behaviour has not been measured here, so it is left drawing the text
    /// as authored, exactly as before. Seven corpus decks state it. Guessing at it would trade a
    /// known-absent feature for a wrong one.
    /// </para>
    /// <para>
    /// The length guard is not defensive padding. Run offsets index this same buffer, so a
    /// casing that changed the length would silently desynchronise every run after it;
    /// <see cref="string.ToUpperInvariant"/> maps one char to one char and cannot, but the guard
    /// states the invariant the indices depend on rather than leaving it to be rediscovered.
    /// Invariant casing is also what keeps a Turkish dotless i from turning into something the
    /// reference did not draw.
    /// </para>
    /// </remarks>
    private static string Capitalised(string content, XElement? runProperties, RunSources sources)
    {
        if (First(runProperties, sources.Defaults, element => Drawing.Attribute(element, "cap"))
            is not "all")
        {
            return content;
        }

        string upper = content.ToUpperInvariant();
        return upper.Length == content.Length ? upper : content;
    }

    private static T? First<T>(XElement? own, XElement?[] defaults, Func<XElement, T?> read)
    {
        if (own is not null && read(own) is { } fromRun) return fromRun;

        foreach (XElement? source in defaults)
        {
            if (source is not null && read(source) is { } value) return value;
        }

        return default;
    }

    /// <summary>
    /// The sources behind one run, split where the shape's own text style goes.
    /// </summary>
    /// <remarks>
    /// <para>
    /// A shape's <c>p:style/a:fontRef</c> is the one rung of the character chain that is not an
    /// <c>a:defRPr</c> anywhere: its colour is a colour reference sitting directly inside the
    /// element and its typefaces are an <c>idx</c> into the theme's font scheme. So it cannot be
    /// appended to <see cref="Defaults"/>, and it needs a place in the order rather than a place
    /// at either end — it <em>beats everything the shape inherits</em> and <em>loses to
    /// everything the body states</em>, which is
    /// <c>TextParagraph::getCharacterStyle</c> (<c>oox/source/drawingml/textparagraph.cxx</c>
    /// :52-67) read from the other end, and exactly what
    /// <see cref="DrawingCharacterStyle.Resolve"/> already does for extraction.
    /// </para>
    /// <para>
    /// Putting it at either extreme is right on every shape that states nothing else, which is
    /// most of them. It is wrong precisely where a deck bothered to state two — a body
    /// placeholder whose <c>a:fontRef</c> names <c>lt1</c> over a master <c>bodyStyle</c> naming
    /// <c>tx1</c> draws white text on a dark panel if the order is right and black text on it if
    /// the shape style loses.
    /// </para>
    /// </remarks>
    /// <param name="Defaults">Every <c>a:defRPr</c> in the chain, most specific first.</param>
    /// <param name="BodyDefaults">
    /// How many of <paramref name="Defaults"/> come from inside the text body — the paragraph's
    /// own <c>a:pPr</c> and the body's <c>a:lstStyle</c> entry for its level. The rest are the
    /// placeholders, the master's <c>p:txStyles</c> and the presentation's default.
    /// </param>
    /// <param name="ShapeStyle">What the shape's <c>p:style/a:fontRef</c> states, or null.</param>
    private readonly record struct RunSources(
        XElement?[] Defaults, int BodyDefaults, DrawingCharacterStyle? ShapeStyle)
    {
        /// <summary>
        /// The first source that states this property, with the shape's own text style consulted
        /// after the body's sources and before everything the shape inherits.
        /// </summary>
        /// <param name="own">The run's own <c>a:rPr</c>, or null.</param>
        /// <param name="fromShape">Reads the property out of the shape's text style.</param>
        /// <param name="read">Reads the property out of one <c>a:rPr</c> or <c>a:defRPr</c>.</param>
        public T? Resolve<T>(
            XElement? own, Func<DrawingCharacterStyle, T?> fromShape, Func<XElement, T?> read)
        {
            if (own is not null && read(own) is { } fromRun) return fromRun;

            for (int i = 0; i < BodyDefaults; i++)
            {
                if (Defaults[i] is { } source && read(source) is { } value) return value;
            }

            if (ShapeStyle is { } style && fromShape(style) is { } stated) return stated;

            for (int i = BodyDefaults; i < Defaults.Length; i++)
            {
                if (Defaults[i] is { } source && read(source) is { } value) return value;
            }

            return default;
        }
    }

    /// <summary>
    /// The one colour a run's fill reduces to, from either an <c>a:solidFill</c> or an
    /// <c>a:gradFill</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>A run's colour is a fill, not a colour</b>, and DrawingML lets it be any of the fills a
    /// shape can have. Text is drawn with one colour, so LibreOffice reduces whichever fill it
    /// finds to a single one on the way in — <c>FillProperties::getBestSolidColor</c>
    /// (<c>oox/source/drawingml/fillproperties.cxx:402-425</c>), reached from
    /// <c>TextCharacterProperties::pushToPropMap</c>
    /// (<c>oox/source/drawingml/textcharacterproperties.cxx:115-117</c>), which then sets
    /// <c>PROP_CharColor</c> at <c>:139</c> and <c>PROP_CharTransparence</c> from that colour's
    /// alpha at <c>:153-156</c>. Reading only <c>a:solidFill</c> left a gradient-filled run with
    /// no stated colour at all, so it inherited whatever was above it and usually came out
    /// opaque black.
    /// </para>
    /// <para>
    /// <b>The alpha is the visible half.</b> Measured on
    /// <c>OnTrac_StarCertificationProgram-3Day.pptx</c>, whose <c>slideMaster3.xml</c> draws an
    /// 82 pt page number from a <c>defRPr</c> whose <c>a:gradFill</c> is two identical <c>tx1</c>
    /// stops at <c>a:alpha val="10000"</c>: the reference emits <c>0 0 0 rg</c> under an
    /// ExtGState of <c>/CA 0.1 /ca 0.1</c>. It is black at a tenth opacity and not a grey, which
    /// is worth stating because it looks like a grey and the two are only the same over a white
    /// background.
    /// </para>
    /// <para>
    /// <b>Which stop.</b> LibreOffice takes the first, unless there are more than two, in which
    /// case it takes the second — its stops are a map keyed by position, so "first" means lowest
    /// and not first in the file. Neither the file order nor
    /// <c>DrawingChartPlot.FillOf</c>'s nearest-to-the-middle rule reproduces that; the chart
    /// heuristic is a deliberate choice for a series colour and is not this.
    /// </para>
    /// </remarks>
    /// <param name="properties">One <c>a:rPr</c> or <c>a:defRPr</c>.</param>
    /// <param name="theme">The theme its scheme colours resolve against.</param>
    private static Colour? RunColour(XElement properties, DrawingTheme? theme)
    {
        if (Drawing.Child(properties, "solidFill") is { } solid)
        {
            foreach (XElement child in solid.Elements())
            {
                if (DrawingColour.Read(child)?.Resolve(theme) is { } colour) return colour;
            }

            return null;
        }

        if (DrawingFill.ReadGradient(Drawing.Child(properties, "gradFill")) is not { Stops.Count: > 0 } gradient)
        {
            return null;
        }

        List<DrawingGradientStop> ordered = [.. gradient.Stops.OrderBy(stop => stop.Position)];
        return ordered[ordered.Count > 2 ? 1 : 0].Colour.Resolve(theme);
    }

    /// <summary>
    /// What a run carrying an <c>a:hlinkClick</c> is drawn in: the theme's <c>hlink</c> slot
    /// under the transforms the run's inherited fill already had.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <b>The slot is swapped, the transform chain is kept.</b> LibreOffice calls
    /// <c>maFillColor.setSchemeClr(XML_hlink)</c> on the character properties it has already
    /// merged (<c>oox/source/drawingml/textrun.cxx:162-166</c>), and <c>Color::setSchemeClr</c>
    /// assigns only the base — <c>meMode</c> and <c>mnC1</c> — leaving the <c>lumMod</c>,
    /// <c>tint</c> and <c>alpha</c> already on the object in place
    /// (<c>oox/source/drawingml/color.cxx:405-413</c>). So a linked run does not simply come out
    /// the theme's link colour; it comes out the theme's link colour <em>seen through whatever
    /// the placeholder was doing to the text colour</em>.
    /// </para>
    /// <para>
    /// That distinction is the whole reason this is not two lines. Measured on
    /// <c>slides/batch-004/pptx/solog_orientation_august_2019.pptx</c> against the banked
    /// 26.2.4.2 reference: its <c>slideLayout1.xml</c> gives the subtitle placeholder
    /// <c>schemeClr tx1</c> under <c>tint val="75000"</c>, which draws the body at
    /// <c>#8B8B8B</c>. The reference draws the three <c>mailto:</c> runs on page 3 at
    /// <c>#8B8BFF</c> — the theme's <c>hlink</c> <c>#0000FF</c> under that same 75% tint, since
    /// tinting toward white leaves a saturated blue channel at FF and lifts the other two to 8B.
    /// Resolving <c>hlink</c> on its own would have drawn them <c>#0000FF</c> and been wrong on
    /// exactly this page while right on the deck's other five, which is the kind of near-miss
    /// that survives a whole round.
    /// </para>
    /// <para>
    /// Only <c>a:solidFill</c> is followed, because <c>maFillColor</c> is the solid-fill colour
    /// and a gradient-filled run leaves it unset — such a run links out as the untransformed
    /// theme colour, which is what LibreOffice does with it too. The shape's
    /// <c>p:style/a:fontRef</c> is skipped in the walk for a different reason: it hands back an
    /// already-resolved <see cref="Colour"/> with no chain left to keep, so there is nothing for
    /// this to carry over even when it is the layer that would have won.
    /// </para>
    /// </remarks>
    private static Colour? HyperlinkColour(
        XElement? runProperties, RunSources sources, DrawingTheme? theme)
    {
        DrawingColour? inherited = sources.Resolve<DrawingColour>(
            runProperties, _ => null, RunFillColour);

        return DrawingColour.FromScheme("hlink", inherited?.Transforms).Resolve(theme);
    }

    /// <summary>
    /// The unresolved <c>a:solidFill</c> colour one <c>a:rPr</c> or <c>a:defRPr</c> states.
    /// </summary>
    /// <remarks>
    /// The same source <see cref="RunColour"/> reads, stopped one step earlier so the transform
    /// chain is still on it. Kept separate rather than folded into <c>RunColour</c> because every
    /// other caller wants the resolved colour and would only have to resolve it again.
    /// </remarks>
    private static DrawingColour? RunFillColour(XElement properties)
    {
        if (Drawing.Child(properties, "solidFill") is not { } solid) return null;

        foreach (XElement child in solid.Elements())
        {
            if (DrawingColour.Read(child) is { } colour) return colour;
        }

        return null;
    }

    private static XElement? LevelStyle(XElement? listStyle, int level)
        => Drawing.Child(listStyle, $"lvl{Math.Clamp(level, 0, 8) + 1}pPr");

    private static TextAlignment Alignment(string? alignment) => alignment switch
    {
        "ctr" => TextAlignment.Centre,
        "r" => TextAlignment.End,
        "just" or "justLow" => TextAlignment.Justify,
        "dist" or "thaiDist" => TextAlignment.Distribute,
        _ => TextAlignment.Start,
    };

    /// <summary>
    /// A <c>a:spcBef</c>/<c>a:spcAft</c> value, which is either points or a percentage.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>a:spcPts</c> states hundredths of a point outright. <c>a:spcPct</c> states thousandths
    /// of a per cent <em>of the paragraph's own character height</em> — not of the line height,
    /// which is what the name suggests and what reading it as a line-spacing rule would give.
    /// LibreOffice resolves it at import against the tallest run in the paragraph and stores the
    /// result as an absolute margin (<c>TextSpacing::toMargin</c>,
    /// <c>oox/inc/drawingml/textspacing.hxx:54</c>, reached from
    /// <c>textparagraphproperties.cxx:438</c>), so it is resolved here for the same reason: by
    /// the time the layouter sees a paragraph it has one spacing, not a rule.
    /// </para>
    /// <para>
    /// <strong>The percentage form is the only one real files use.</strong> Of the 324
    /// <c>a:pPr</c> in the baked diagram drawings of LibreOffice's <c>sd/qa</c> corpus, all 324
    /// state their spacing as a percentage and none in points — so ignoring it set every
    /// multi-paragraph node's lines tighter than the reference.
    /// </para>
    /// </remarks>
    /// <param name="spacing">The <c>a:spcBef</c> or <c>a:spcAft</c> element.</param>
    /// <param name="characterHeight">The tallest run in the paragraph, which a percentage scales.</param>
    private static Length Spacing(XElement? spacing, Length characterHeight)
    {
        if (Drawing.Number(Drawing.Child(spacing, "spcPts"), "val") is { } points && points > 0)
        {
            return Length.FromEmu(points * Length.EmuPerPoint / 100);
        }

        if (Drawing.Number(Drawing.Child(spacing, "spcPct"), "val") is not { } percent
            || percent <= 0)
        {
            return Length.Zero;
        }

        // A paragraph with no run of its own is spaced against twelve points, the size
        // LibreOffice falls back to when the paragraph style states no character height.
        double size = characterHeight > Length.Zero ? characterHeight.Points : 12.0;

        // Hundredths of a point, truncated exactly where LibreOffice truncates: the product is
        // cast to an integer before it leaves points.
        int hundredths = (int)(size * percent / 1000.0);
        return Length.FromEmu(hundredths * Length.EmuPerPoint / 100);
    }

    /// <summary>A <c>a:lnSpc</c>, as a percentage of the line height or as an exact height.</summary>
    /// <remarks>
    /// <para>
    /// <c>a:spcPct</c> states thousandths of a per cent, and the reference keeps only the
    /// <strong>whole per cent</strong>: <c>TextSpacing::toLineSpacing</c> is
    /// <c>static_cast&lt;sal_Int16&gt;(nValue / 1000)</c> in integer arithmetic
    /// (<c>oox/inc/drawingml/textspacing.hxx:52</c>), and EditEngine then holds a
    /// <c>sal_uInt16</c> percentage throughout. So a file stating <c>131579</c> is laid out at
    /// 131 per cent, not 131.579, and the truncation belongs here at import rather than in the
    /// layout that consumes it.
    /// </para>
    /// <para>
    /// Narrow reach, and stated as such: across the whole slides track only <strong>39
    /// occurrences over seven values</strong> are not already a whole per cent
    /// (<c>research/probes/slides-r16/lnspc-scan.py</c>). It is here because it is what the
    /// reference holds, not because it moves the corpus.
    /// </para>
    /// </remarks>
    private static LineSpacingRule LineSpacing(XElement? spacing)
    {
        if (Drawing.Number(Drawing.Child(spacing, "spcPct"), "val") is { } percent && percent > 0)
            return LineSpacingRule.Multiple(Math.Truncate(percent / 1000.0) / 100.0);

        if (Drawing.Number(Drawing.Child(spacing, "spcPts"), "val") is { } points && points > 0)
            return LineSpacingRule.Exactly(Length.FromEmu(points * Length.EmuPerPoint / 100));

        return LineSpacingRule.SingleSpaced;
    }

    private static double Thousandth(XElement? element, string attribute, double whenAbsent)
        => Drawing.Number(element, attribute) is { } value && value > 0
            ? value / 100000.0
            : whenAbsent;

    private static long Emu(string? stated, long whenAbsent)
        => long.TryParse(stated, NumberStyles.Integer, CultureInfo.InvariantCulture, out long value)
            ? value
            : whenAbsent;

    /// <summary>An EMU-valued attribute from the first source in the chain to state it.</summary>
    private static long Emu(List<XElement> chain, string attribute)
        => long.TryParse(
            Stated(chain, attribute), NumberStyles.Integer,
            CultureInfo.InvariantCulture, out long value)
            ? value
            : 0;
}
