using System.Globalization;
using System.Text;
using System.Xml.Linq;
using XRay.Office.Core.Geometry;
using XRay.Office.Core.Graphics;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.Presentations.Layout;
using XRay.Office.Text.Fonts;
using XRay.Office.Text.Layout;

namespace XRay.Office.Presentations.OpenDocument;

/// <summary>
/// Reads a shape's <c>text:p</c> children into the paragraphs slide layout takes.
/// </summary>
/// <remarks>
/// The ODF counterpart of <c>PptxTextBody</c>. What differs is where the formatting lives: none
/// of it is on the run, all of it is in a style reached by name, and the run's own
/// <c>text:style-name</c> is only the innermost link of a chain that starts at the shape's
/// graphic style. <see cref="OdfTextFormat.Resolve"/> already walks exactly that chain, so this
/// is mostly assembling the cascade and turning what comes back into layout's own vocabulary.
/// </remarks>
internal static class OdfTextBody
{
    /// <summary>The size a run gets when nothing in its cascade states one: 18 pt.</summary>
    /// <remarks>
    /// Impress's own default for a text frame. A word processor's default is 12 pt, and using
    /// that here would make every unstated line of every slide a third too small.
    /// </remarks>
    private static readonly Length DefaultSize = Length.FromPoints(18);

    /// <summary>The character a <c>text:line-break</c> becomes.</summary>
    private const char LineSeparator = '\u2028';

    /// <summary>Reads a shape's paragraphs.</summary>
    /// <param name="file">The document, for its styles.</param>
    /// <param name="paragraphs">The <c>text:p</c> elements.</param>
    /// <param name="shapeCascade">
    /// The shape's own style references, which the paragraph and run styles sit inside.
    /// </param>
    /// <param name="fields">
    /// What the slide this text is drawn on answers for a header, footer, date-time or
    /// slide-number field. Null leaves every field showing the characters the file stores
    /// against it, which is what a document with no page to resolve against can say.
    /// </param>
    public static SlideTextBody Read(
        OdfFile file,
        IEnumerable<XElement> paragraphs,
        IReadOnlyList<OdfStyleReference> shapeCascade,
        OdpRunningObjects? fields = null)
    {
        ArgumentNullException.ThrowIfNull(file);
        ArgumentNullException.ThrowIfNull(paragraphs);
        ArgumentNullException.ThrowIfNull(shapeCascade);

        List<SlideParagraph> read = [];
        bool fontIndependent = false;
        string? outlineBase = OutlineStyleBase(file, shapeCascade);

        foreach (XElement paragraph in paragraphs)
        {
            OdfStyleReference style = new(
                paragraph.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value,
                OdfStyleFamily.Paragraph);

            int level = OutlineLevel(paragraph);

            List<OdfStyleReference> cascade =
                [.. shapeCascade, OutlineStyle(outlineBase, level), style];

            if (file.Styles.ResolveProperty(
                    cascade, OdfPropertyKind.Paragraph,
                    OdfNamespaces.Style, "font-independent-line-spacing").AsBoolean() == true)
            {
                fontIndependent = true;
            }

            SlideParagraph read1 = Paragraph(file, paragraph, cascade, fields);

            read.Add(Label(file, paragraph, level) is { } label
                ? read1 with
                {
                    StartIndent = label.Start,
                    FirstLineIndent = label.Hanging,
                    Marker = label.Marker,

                    // ODF takes the level from the list nesting and suppresses nothing when the
                    // paragraph is empty, so the numbering format — and the bullet box that
                    // floors an empty line — survives. See SlideParagraph.EmptyKeepsMarkerLevel.
                    EmptyKeepsMarkerLevel = true,
                }
                : read1);
        }

        return new SlideTextBody
        {
            Paragraphs = read,
            Insets = Insets(file, shapeCascade),
            Anchor = Anchor(file, shapeCascade),
            AutoFit = Shrinks(file, shapeCascade),
            FontIndependentLineSpacing = fontIndependent,
        };
    }

    /// <summary>
    /// A list item's label: where its text starts, how far its marker hangs back, and what it is.
    /// </summary>
    private readonly record struct ListLabel(Length Start, Length Hanging, SlideMarker? Marker);

    /// <summary>
    /// How deeply a paragraph is nested in <c>text:list</c> elements, counted from one.
    /// </summary>
    /// <remarks>
    /// ODF states list <em>structure</em> by nesting and list <em>appearance</em> by a separately
    /// named style, which is the opposite way round from OOXML's flat "paragraph plus a level
    /// number". So the level is the nesting depth, and zero means the paragraph is not in a list
    /// at all — in which case its own <c>fo:margin-left</c> decides and nothing here applies.
    /// </remarks>
    private static int OutlineLevel(XElement paragraph)
    {
        int level = 0;

        for (XElement? ancestor = paragraph.Parent; ancestor is not null;
             ancestor = ancestor.Parent)
        {
            if (ancestor.Name.NamespaceName == OdfNamespaces.Text
                && ancestor.Name.LocalName == "list")
            {
                level++;
            }
        }

        return level;
    }

    /// <summary>
    /// The indents and the marker a list level gives its items.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>ODF's default label geometry is not <c>fo:margin-left</c> and
    /// <c>fo:text-indent</c>.</strong> Those belong to the ODF 1.2 <em>label-alignment</em> mode,
    /// which LibreOffice writes for Writer; a presentation's list style uses the older
    /// <em>label-width-and-position</em> mode, whose two quantities are <c>text:space-before</c>
    /// and <c>text:min-label-width</c>. The text starts at the sum of them and the marker at the
    /// space alone, which is exactly the <c>marL</c> and <c>marL + indent</c> pair PresentationML
    /// states directly. Measured on <c>slides-features.odp</c>, whose level 1 states no space and
    /// a 0.6 cm label: LibreOffice draws the bullet at 56.693 and the text at 73.701, and 17.008 pt
    /// is 0.6 cm.
    /// </para>
    /// <para>
    /// The label's own size is a percentage of the item's text — 45% in every deck LibreOffice
    /// writes — and its face is the level's, which is a symbol font that will not be installed and
    /// will substitute. Both come from the level's <c>style:text-properties</c> rather than from
    /// the paragraph's.
    /// </para>
    /// </remarks>
    private static ListLabel? Label(OdfFile file, XElement paragraph, int level)
    {
        if (level < 1) return null;
        if (ListStyle(file, paragraph) is not { } style) return null;
        if (style.GetLevel(level) is not { } definition) return null;

        Length space = Measure(definition, "space-before");
        Length label = Measure(definition, "min-label-width");

        return new ListLabel(space + label, -label, Marker(file, definition, level, style));

        static Length Measure(OdfListLevel definition, string name)
            => OdfValue.ParseLength(
                   definition.LevelProperties?.Get(OdfNamespaces.Text, name))
               ?? Length.Zero;
    }

    /// <summary>
    /// The list style a paragraph's innermost <c>text:list</c> names.
    /// </summary>
    /// <remarks>
    /// Innermost first, then outwards: LibreOffice writes the style on the outermost list of a
    /// run and leaves the nested ones bare, so a reader taking only the immediate parent finds
    /// nothing on every level past the first.
    /// </remarks>
    private static OdfListStyle? ListStyle(OdfFile file, XElement paragraph)
    {
        for (XElement? ancestor = paragraph.Parent; ancestor is not null;
             ancestor = ancestor.Parent)
        {
            if (ancestor.Name.NamespaceName != OdfNamespaces.Text) continue;
            if (ancestor.Name.LocalName != "list") continue;

            string? name = ancestor.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value;
            if (file.Styles.FindListStyle(name) is { } style) return style;
        }

        return null;
    }

    /// <summary>
    /// The marker a level draws, or null when it draws none.
    /// </summary>
    /// <remarks>
    /// Only a bullet, deliberately. A numbered level needs a counter carried across the body and
    /// restarted where the level rises, and inventing "1." for every item is a worse answer than
    /// none — the same decision the OOXML path records for <c>a:buAutoNum</c>. The counters passed
    /// here are all ones so that <see cref="OdfListStyle.FormatLabel"/> returns the bullet without
    /// pretending to number anything.
    /// </remarks>
    private static SlideMarker? Marker(
        OdfFile file, OdfListLevel definition, int level, OdfListStyle style)
    {
        if (definition.Kind != OdfListLabelKind.Bullet) return null;
        if (style.FormatLabel(level, [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]) is not { Length: > 0 } text)
            return null;

        string? family = Family(file, definition.Typeface);

        // A recodeable face keeps its Private Use Area slot, because `SlideTextLayout` needs the
        // slot and the family together to reach the OpenSymbol glyph holding the same picture.
        //
        // `FormatLabel` is the *extraction* answer and collapses the slot to U+2022
        // (`OutlineNumbers.NormaliseBullet`), which is right for an index — a Wingdings slot means
        // nothing to a consumer — and wrong for a rendering, where it turned every green check
        // mark on `redac-sas-201509-asisp-research.odp` page 7 into a black dot. The deck reader
        // reached the same fork from the other side and took the other branch
        // (`PptxTextBody.Marked`); this is the ODF statement of it.
        //
        // Censused over the converted corpus: **3598 bullet levels in 74 of the 302 `.odp` state a
        // Private Use Area character**, every one of them in the F000 block, and every family they
        // name has a recode table but one — `CommonBullets`, once.
        if (definition.BulletCharacter is { Length: 1 } slot
            && SymbolFontRecode.IsRecodeable(family))
        {
            text = definition.Prefix + slot + definition.Suffix;
        }

        return new SlideMarker(text, family, definition.RelativeSize ?? 1.0, definition.Colour);
    }

    /// <summary>
    /// The name a presentation's per-level outline styles share, or null when the shape has none.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>An outline placeholder's formatting is per level and lives in a style the shape
    /// only reaches through its parent.</strong> LibreOffice creates one presentation style per
    /// master page and outline level — <c>Default-outline1</c> … <c>Default-outline9</c>,
    /// chained parent to child — and the shape's own <c>presentation:style-name</c> inherits from
    /// level one (<c>xmloff/source/draw/ximpstyl.cxx</c>'s <c>ImpSetGraphicStyles</c>). So a
    /// paragraph at level two must resolve against <c>Default-outline2</c>, which nothing in the
    /// shape's own cascade points at.
    /// </para>
    /// <para>
    /// What it carries is the font size per level and the space above a paragraph. Measured on
    /// <c>slides-features.odp</c>: <c>Default-outline2</c> states <c>fo:margin-top="0.4cm"</c> and
    /// nothing else, and without it the deck's third outline paragraph sat 11.23 pt above where
    /// LibreOffice draws it — a drift that grows with every level-two paragraph and looks like a
    /// line-height bug.
    /// </para>
    /// </remarks>
    private static string? OutlineStyleBase(
        OdfFile file, IReadOnlyList<OdfStyleReference> cascade)
    {
        foreach (OdfStyleReference reference in cascade)
        {
            if (reference.Family != OdfStyleFamily.Presentation) continue;

            string? name = reference.Name;
            for (int depth = 0; name is not null && depth < OdfStyles.MaxParentChainDepth; depth++)
            {
                if (name.Length > 8
                    && char.IsAsciiDigit(name[^1])
                    && name.AsSpan(..^1).EndsWith("-outline", StringComparison.Ordinal))
                {
                    return name[..^1];
                }

                name = file.Styles.Find(name, OdfStyleFamily.Presentation)?.ParentStyleName;
            }
        }

        return null;
    }

    private static OdfStyleReference OutlineStyle(string? outlineBase, int level)
        => outlineBase is null || level < 1
            ? new OdfStyleReference(null, OdfStyleFamily.Presentation)
            : new OdfStyleReference(
                outlineBase + Math.Clamp(level, 1, 9).ToString(CultureInfo.InvariantCulture),
                OdfStyleFamily.Presentation);

    /// <summary>
    /// The text insets, which ODF spells as the shape's padding.
    /// </summary>
    /// <remarks>
    /// Zero when the style states none, unlike DrawingML — ODF has no implied default here, and a
    /// LibreOffice-written shape states all four. A deck converted from PPTX carries the OOXML
    /// defaults written out explicitly, which is how the two paths agree on the same document.
    /// </remarks>
    private static Margins Insets(OdfFile file, IReadOnlyList<OdfStyleReference> cascade) => new(
        Padding(file, cascade, "padding-left"),
        Padding(file, cascade, "padding-top"),
        Padding(file, cascade, "padding-right"),
        Padding(file, cascade, "padding-bottom"));

    private static Length Padding(
        OdfFile file, IReadOnlyList<OdfStyleReference> cascade, string name)
        => file.Styles.ResolveProperty(
               cascade, OdfPropertyKind.Graphic, OdfNamespaces.FoCompatible, name)
               .AsLength()
           ?? Length.Zero;

    /// <summary>
    /// Whether the shape shrinks its text until it fits — Impress's <em>autofit</em>, and
    /// <see cref="SlideTextBody.AutoFit"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <strong>ODF spells one property two ways and they have to be read together.</strong>
    /// <c>drawing::TextFitToSizeType</c> has four values and <c>sdpropls.cxx</c>:143-144 maps
    /// <em>both</em> <c>draw:fit-to-size</c> and <c>style:shrink-to-fit</c> onto it with
    /// <c>MID_FLAG_MERGE_PROPERTY</c>: <c>draw:fit-to-size</c> carries
    /// <c>false</c> / <c>true</c> (proportional) / <c>all</c> (all lines) /
    /// <c>shrink-to-fit</c> (autofit) through <c>pXML_FitToSize_Enum</c> (:677-684), and
    /// <c>style:shrink-to-fit</c> carries the autofit bit alone through
    /// <c>pXML_ShrinkToFit_Enum</c> (:686-693). The second exists because the first is ODF 1.2's
    /// spelling and older consumers read <c>true</c> as <em>stretch</em>; LibreOffice therefore
    /// writes <c>draw:fit-to-size="false" style:shrink-to-fit="true"</c> for an autofitted shape,
    /// and a reader that consults only the first concludes the shape does not autofit.
    /// </para>
    /// <para>
    /// Which is what happened here: <c>SlideAutofit</c> has been a full port of
    /// <c>autoFitTextForCompatibility</c> since round 52 and the ODF path reached none of it. It
    /// is not a rare attribute — <c>style:shrink-to-fit="true"</c> appears <strong>3515 times,
    /// in all 302</strong> of the converted corpus's <c>.odp</c>.
    /// </para>
    /// <para>
    /// Only the autofit value is honoured. <c>PROPORTIONAL</c> and <c>ALLLINES</c> — Impress's
    /// <em>fit to frame</em>, which stretches the glyphs rather than choosing a smaller size —
    /// are a different transform and are not modelled on any of the three presentation readers;
    /// the corpus states neither, all 25 525 of its <c>draw:fit-to-size</c> saying
    /// <c>false</c>.
    /// </para>
    /// </remarks>
    private static bool Shrinks(OdfFile file, IReadOnlyList<OdfStyleReference> cascade)
    {
        if (file.Styles.ResolveProperty(
                cascade, OdfPropertyKind.Graphic, OdfNamespaces.Style, "shrink-to-fit").Is("true"))
        {
            return true;
        }

        return file.Styles.ResolveProperty(
            cascade, OdfPropertyKind.Graphic, OdfNamespaces.Draw, "fit-to-size").Is("shrink-to-fit");
    }

    private static TextAnchor Anchor(OdfFile file, IReadOnlyList<OdfStyleReference> cascade)
    {
        OdfProperty alignment = file.Styles.ResolveProperty(
            cascade, OdfPropertyKind.Graphic, OdfNamespaces.Draw, "textarea-vertical-align");

        if (alignment.Is("middle")) return TextAnchor.Middle;
        if (alignment.Is("bottom")) return TextAnchor.Bottom;
        return TextAnchor.Top;
    }

    private static SlideParagraph Paragraph(
        OdfFile file,
        XElement paragraph,
        IReadOnlyList<OdfStyleReference> cascade,
        OdpRunningObjects? fields)
    {
        StringBuilder text = new();
        List<SlideTextRun> runs = [];
        IReadOnlyList<OdfStyleReference>? lastSpan = null;

        Collect(file, paragraph, cascade, text, runs, fields, ref lastSpan);

        if (runs.Count == 0)
        {
            // An empty paragraph is still a line, and it is as tall as the text that *would* go
            // on it -- which is the formatting of its last empty `text:span`, not the
            // paragraph's own. See `EmptyLineCascade` for the measurement.
            runs.Add(Run(file, lastSpan ?? cascade, 0, 0));
        }

        return new SlideParagraph(
            text.ToString(),
            runs,
            Alignment(file, cascade),
            Spacing(file, cascade, "margin-top"),
            Spacing(file, cascade, "margin-bottom"),
            LineSpacing(file, cascade),
            Spacing(file, cascade, "margin-left"),
            Spacing(file, cascade, "text-indent"),
            Language: null);
    }

    /// <summary>
    /// Walks a paragraph's children, appending text and one run per style change.
    /// </summary>
    /// <remarks>
    /// Spans nest, so the cascade grows as the walk descends and each nested span's formatting
    /// resolves through every span above it. Flattening to the innermost style instead would lose
    /// the bold of a bold span containing a coloured one.
    /// </remarks>
    private static void Collect(
        OdfFile file,
        XElement element,
        IReadOnlyList<OdfStyleReference> cascade,
        StringBuilder text,
        List<SlideTextRun> runs,
        OdpRunningObjects? fields,
        ref IReadOnlyList<OdfStyleReference>? lastSpan)
    {
        foreach (XNode node in element.Nodes())
        {
            if (node is XText literal)
            {
                if (literal.Value.Length == 0) continue;

                runs.Add(Run(file, cascade, text.Length, literal.Value.Length));
                text.Append(literal.Value);
                continue;
            }

            if (node is not XElement child) continue;

            if (child.Name.NamespaceName == OdfNamespaces.Presentation)
            {
                // A running object's field. Its element is empty in the file: what it draws is
                // the declaration the *slide* names, so nothing here can be read off the frame.
                if (Field(child.Name.LocalName, fields) is { Length: > 0 } declared)
                {
                    runs.Add(Run(file, cascade, text.Length, declared.Length));
                    text.Append(declared);
                }

                continue;
            }

            if (child.Name.NamespaceName != OdfNamespaces.Text) continue;

            switch (child.Name.LocalName)
            {
                case "span":
                    IReadOnlyList<OdfStyleReference> nested =
                    [
                        .. cascade,
                        new OdfStyleReference(
                            child.Attribute(XName.Get("style-name", OdfNamespaces.Text))?.Value,
                            OdfStyleFamily.Text),
                    ];

                    // Recorded whether or not the span carries any text, because an empty one
                    // still sizes the line: see `EmptyLineCascade`. Recorded *before* the
                    // descent, so that the winner is the last span entered in document order --
                    // the innermost of a nest, and the later of two siblings. Both were
                    // measured; the nest is the one an "outermost" or "largest" rule gets wrong.
                    lastSpan = nested;
                    Collect(file, child, nested, text, runs, fields, ref lastSpan);
                    break;

                // The slide's own number. The element's content is the placeholder the file
                // stores against it -- LibreOffice writes the literal string `<number>` -- so
                // drawing that content is drawing the placeholder rather than the field.
                case "page-number" when fields is not null && Current(child):
                    runs.Add(Run(file, cascade, text.Length, fields.PageNumber.Length));
                    text.Append(fields.PageNumber);
                    break;

                case "s":
                    // A run of spaces, collapsed in the file and expanded here: text:c says how
                    // many, and its absence means one.
                    int count = int.TryParse(
                        child.Attribute(XName.Get("c", OdfNamespaces.Text))?.Value,
                        out int stated) ? Math.Clamp(stated, 1, 4096) : 1;

                    runs.Add(Run(file, cascade, text.Length, count));
                    text.Append(' ', count);
                    break;

                case "tab":
                    runs.Add(Run(file, cascade, text.Length, 1));
                    text.Append('\t');
                    break;

                case "line-break":
                    text.Append(LineSeparator);
                    break;

                // A hyperlink, and in a *draw* shape's text that is a field rather than a
                // character property. `txtparai.cxx`:1352-1370 asks the cursor for a
                // `HyperLinkURL` property and builds an `XMLImpHyperlinkContext_Impl` when it has
                // one and an `XMLUrlFieldImportContext` when it does not — Writer's text cursor has
                // it and Draw's, Impress's and Calc's do not, so every slide's `text:a` becomes a
                // `com.sun.star.text.TextField.URL` whose `Representation` is the element's own
                // content (`txtfldi.cxx`:2907-2918).
                //
                // Two things follow and neither is visible in the formatting: the field is one
                // portion, so an over-long one is filled to the *character* rather than moved down
                // or broken at a separator, and the lines it spills onto are stacked one ascent
                // apart. Both live in `SlideTextLayout`; all that is read here is which characters
                // are the field.
                //
                // The content is flattened, which is what the reference does — a
                // `XMLTextFieldImportContext` overrides `characters` and nothing else, so a
                // `text:span` nested inside a `text:a` contributes no formatting of its own. No
                // document of the converted corpus has one: 665 `text:a` in 156 of the 302 `.odp`
                // and not a single nested span.
                case "a":
                    string linked = child.Value;
                    if (linked.Length == 0) break;

                    runs.Add(Run(file, cascade, text.Length, linked.Length) with { IsField = true });
                    text.Append(linked);
                    break;

                default:
                    // A field, a bookmark, a note anchor: whatever text it carries is its own.
                    Collect(file, child, cascade, text, runs, fields, ref lastSpan);
                    break;
            }
        }
    }

    /// <summary>
    /// What a <c>presentation:header</c>, <c>-footer</c> or <c>-date-time</c> field draws on
    /// this slide, or null when it draws nothing.
    /// </summary>
    /// <remarks>
    /// These three elements are always empty in the file. The text is the document-level
    /// declaration the slide names through <c>presentation:use-footer-name</c> and its siblings —
    /// <c>SdXMLGenericPageContext::endFastElement</c> (<c>xmloff/source/draw/ximppage.cxx</c>:301-360)
    /// copies it onto the page, and <c>SdModule::CalcFieldValueHdl</c>
    /// (<c>sd/source/ui/app/sdmod2.cxx</c>:374-425) is what reads it back for the field. A slide
    /// naming no declaration leaves the field empty, which is why a master's footer can be present
    /// on a page and still draw nothing.
    /// </remarks>
    private static string? Field(string name, OdpRunningObjects? fields) => name switch
    {
        "header" => fields?.Header,
        "footer" => fields?.Footer,
        "date-time" => fields?.DateTime,
        _ => null,
    };

    /// <summary>
    /// Whether a <c>text:page-number</c> asks for the page it is on rather than its neighbour.
    /// </summary>
    /// <remarks>
    /// <c>text:select-page</c> takes <c>previous</c>, <c>current</c> and <c>next</c>, and absent
    /// is <c>current</c>. Only the current page is substituted here: the other two are a
    /// word-processing construct that no presentation LibreOffice writes uses, and drawing the
    /// stored placeholder for them is at least visibly a placeholder.
    /// </remarks>
    private static bool Current(XElement field)
    {
        string? select = field.Attribute(XName.Get("select-page", OdfNamespaces.Text))?.Value;
        return select is null or "current";
    }

    /// <summary>
    /// Why an empty paragraph's line is measured against its last <c>text:span</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// LibreOffice's export writes an empty line as <c>&lt;text:p&gt;&lt;text:span
    /// text:style-name="T15"/&gt;&lt;/text:p&gt;</c> — a span carrying the character formatting
    /// and no characters — and EditEngine measures such a line from the character attributes at
    /// the paragraph's own position: <c>ImpEditEngine::CreateLines</c> builds a dummy portion
    /// from <c>SeekCursor(rParaPortion.GetNode(), 0, aTmpFont)</c> and gives it
    /// <c>ImplCalculateFontIndependentLineSpacing(aTmpFont.GetFontHeight())</c>
    /// (<c>editeng/source/editeng/impedit3.cxx</c>:1896-1902), so the empty span's own size is
    /// what sets the height.
    /// </para>
    /// <para>
    /// Reading the paragraph's cascade instead gives the shape's <em>default</em> size, which on
    /// a presentation placeholder is far larger than the run it stands in for. Measured on
    /// <c>0335fab9-79f0-4944-b92c-f223837ca2d8.odp</c> against 26.2.4.2, one 16 pt empty span
    /// between two 16 pt paragraphs: the reference advances 23.19 pt over it and we advanced
    /// 42.41, because the placeholder's default is 32 pt and 1.2 × (32 − 16) is 19.2 — the
    /// "constant excess on every inter-paragraph gap" that a visual reading of that slide
    /// reported. A <em>bare</em> <c>&lt;text:p/&gt;</c> with no span at all does take the
    /// paragraph's default, and both renderers agree on it at 42.41.
    /// </para>
    /// <para>
    /// The span that wins is the last one <em>entered</em>, not the largest and not the
    /// outermost: over four one-attribute variants of the same slide, 26.2.4.2 answers 8 pt for
    /// a 40 pt span followed by an 8 pt one, 40 pt for the reverse order, and 8 pt for an 8 pt
    /// span nested inside a 40 pt one. See <c>probes/odp-embed-r79/</c>.
    /// </para>
    /// </remarks>
    private static SlideTextRun Run(
        OdfFile file, IReadOnlyList<OdfStyleReference> cascade, int start, int length)
    {
        OdfTextFormat format = OdfTextFormat.Resolve(file.Styles, cascade);

        return new SlideTextRun(
            start,
            length,
            Family(file, format.FontName),
            format.FontSize ?? DefaultSize,
            format.IsBold ? 700 : 400,
            format.IsItalic,
            format.Colour ?? Colour.Black,
            // `style:text-underline-style` and `style:text-line-through-style`, which
            // `OdfTextFormat` has resolved since it was written and which nothing passed on: a
            // slide's hyperlinks came out the right colour with no rule under them. ODF states
            // the decoration explicitly on the link's own text style -- LibreOffice's export
            // writes `style:text-underline-style="solid"` beside `fo:color` -- so there is no
            // implicit "a hyperlink is underlined" rule here as there is in DrawingML.
            IsUnderlined: format.IsUnderlined,
            IsStruckThrough: format.IsStruckThrough,
            Escapement: Escaped(format.Position));
    }

    /// <summary>
    /// The rise and shrink an ODF text position asks for.
    /// </summary>
    /// <remarks>
    /// <c>style:text-position</c> can state both numbers and <see cref="OdfTextFormat"/> keeps
    /// only the direction, so this is LibreOffice's automatic pair: 58% of the size, raised or
    /// lowered by <c>0.8 × (100 − 58)</c> of it (<c>editeng/source/items/svxfont.cxx:85-91</c>,
    /// which is where <c>DFLT_ESC_SUPER</c>'s 33 comes from).
    /// </remarks>
    private static SlideEscapement Escaped(OdfTextPosition position) => position switch
    {
        OdfTextPosition.Superscript => new SlideEscapement(33, SlideEscapement.AutomaticProportion),
        OdfTextPosition.Subscript => new SlideEscapement(-33, SlideEscapement.AutomaticProportion),
        _ => SlideEscapement.None,
    };

    /// <summary>
    /// The family name a <c>style:font-name</c> refers to.
    /// </summary>
    /// <remarks>
    /// ODF names a <em>declaration</em> rather than a family, and the declaration's
    /// <c>svg:font-family</c> is a CSS-style list that may be quoted. Passing the declaration's
    /// own name to a font resolver works only when the two happen to coincide, which is common
    /// enough to hide the bug and not universal.
    /// </remarks>
    private static string? Family(OdfFile file, string? fontName)
    {
        if (fontName is null) return null;

        // `fo:font-family` states the list directly and `style:font-name` names a declaration that
        // holds one, so the same trimming has to happen on both paths: a level writing
        // `fo:font-family="&apos;Wingdings 2&apos;"` — 721 of the converted corpus's bullet levels
        // do — asks for a family whose name is not `'Wingdings 2'`.
        string family = file.Styles.FontFaces.TryGetValue(fontName, out OdfFontFace? face)
                        && !string.IsNullOrEmpty(face.FontFamily)
            ? face.FontFamily
            : fontName;

        int comma = family.IndexOf(',', StringComparison.Ordinal);
        if (comma >= 0) family = family[..comma];

        return family.Trim().Trim('\'', '"');
    }

    /// <summary>
    /// A paragraph measurement stated as an <c>fo:</c> length, or zero.
    /// </summary>
    /// <remarks>
    /// The indents and the space around a paragraph, which ODF spells as margins on the paragraph
    /// itself. Not the ones an <em>outline</em> paragraph gets: those come from the list style its
    /// <c>text:list</c> names, which is a different resolution path and is the open item recorded
    /// in the TODO.
    /// </remarks>
    private static Length Spacing(
        OdfFile file, IReadOnlyList<OdfStyleReference> cascade, string name)
        => file.Styles.ResolveProperty(
               cascade, OdfPropertyKind.Paragraph, OdfNamespaces.FoCompatible, name)
               .AsLength()
           ?? Length.Zero;

    private static TextAlignment Alignment(OdfFile file, IReadOnlyList<OdfStyleReference> cascade)
    {
        OdfProperty alignment = file.Styles.ResolveProperty(
            cascade, OdfPropertyKind.Paragraph, OdfNamespaces.FoCompatible, "text-align");

        if (alignment.Is("center")) return TextAlignment.Centre;
        if (alignment.Is("end") || alignment.Is("right")) return TextAlignment.End;
        if (alignment.Is("justify")) return TextAlignment.Justify;
        return TextAlignment.Start;
    }

    private static LineSpacingRule LineSpacing(
        OdfFile file, IReadOnlyList<OdfStyleReference> cascade)
    {
        OdfProperty height = file.Styles.ResolveProperty(
            cascade, OdfPropertyKind.Paragraph, OdfNamespaces.FoCompatible, "line-height");

        if (height.AsPercentage() is { } proportion && proportion > 0)
            return LineSpacingRule.Multiple(proportion);

        if (height.AsLength() is { } exact && exact > Length.Zero)
            return LineSpacingRule.Exactly(exact);

        return LineSpacingRule.SingleSpaced;
    }
}
