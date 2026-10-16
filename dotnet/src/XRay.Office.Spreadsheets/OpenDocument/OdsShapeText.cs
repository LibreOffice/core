using System.Xml.Linq;
using XRay.Office.Core.Units;
using XRay.Office.OpenDocument;
using XRay.Office.OpenDocument.Styles;
using XRay.Office.Spreadsheets.Layout;

namespace XRay.Office.Spreadsheets.OpenDocument;

/// <summary>
/// The text inside a drawing shape anchored on an ODF sheet.
/// </summary>
/// <remarks>
/// <para>
/// <strong>A text box on a Calc sheet is a drawing object, not a cell.</strong> ODF anchors one
/// inside the <c>table:table-cell</c> it is fastened to, which is what made its paragraphs look
/// like the cell's — and Calc reads it through the drawing layer, prints it with
/// <c>PrintDrawingLayer</c>, and never lets a cell question see it. So it needs a
/// <see cref="SheetShapeText"/> of its own, exactly as the SpreadsheetML and BIFF readers already
/// build one; see <see cref="XRay.Office.Core.Extraction.ContentTableCell.GetOwnText"/> for the
/// other half of the same rule.
/// </para>
/// <para>
/// <strong>Which style says what, and two of the four do not say anything.</strong> The box
/// properties — the insets, the two text-area adjustments, the wrap and the overflow behaviour —
/// come from the shape's <c>draw:style-name</c> graphic style. The <em>character</em> properties
/// come from the paragraph's own <c>text:style-name</c> and its spans' and from nowhere else: on a
/// Calc sheet neither the graphic style's <c>style:text-properties</c> nor the
/// <c>draw:text-style-name</c> paragraph style reaches a run at all, and a run inheriting nothing
/// is set in the drawing layer's own pool default rather than in either of them.
/// </para>
/// <para>
/// That is measured rather than derived, on <c>features/sheet-shape-text.fods</c> through
/// 26.2.4.2 with one attribute changed at a time: removing the graphic style's
/// <c>fo:font-size="18pt"</c>, changing it to 8 pt, putting <c>fo:font-size="14pt"</c> on the
/// <c>draw:text-style-name</c> style and removing <c>draw:text-style-name</c> outright all leave
/// the rendering byte for byte identical, and the paragraph that names no style of its own is
/// drawn at <strong>11.99 pt in Liberation Serif</strong> in every one of them — the EditEngine
/// pool's 12 pt and <c>DefaultFontType::LATIN_TEXT</c>, which is what
/// <see cref="SheetShapeText.DefaultSize"/> and <see cref="SheetShapeText.DefaultFamily"/> already
/// carry. The control is the same paragraph naming that style <em>directly</em>, which does move:
/// it is drawn centred. <c>SdXMLShapeContext::SetStyle</c> resolves
/// <c>draw:text-style-name</c> in the shape import's own automatic-styles context
/// (<c>xmloff/source/draw/ximpshap.cxx</c>:740-757), and Calc's import registers its paragraph
/// automatic styles elsewhere.
/// </para>
/// <para>
/// <strong>The insets are zero when nothing states them</strong>, which is not DrawingML's tenth
/// and twentieth of an inch: <c>SDRATTR_TEXT_LEFTDIST</c> and its three siblings default to 0 in
/// the drawing layer's own pool (<c>svx/source/svdraw/svdattr.cxx</c>:247-250). Every shape
/// LibreOffice's ODF export writes states all four, so the default is reached only by a
/// hand-written file.
/// </para>
/// </remarks>
internal static class OdsShapeText
{
    /// <summary>The size a run inherits nothing from anywhere is set at.</summary>
    /// <remarks>
    /// The EditEngine pool's own default, which is what a Calc drawing object's text cursor
    /// reports — the same 12 pt <see cref="SheetShapeText.DefaultSize"/> records for the
    /// SpreadsheetML path, and for the same reason.
    /// </remarks>
    private static Length DefaultSize => SheetShapeText.DefaultSize;

    /// <summary>The two spellings of a face, in the order one style's own attributes settle.</summary>
    /// <remarks>
    /// <c>style:font-name</c> and <c>fo:font-family</c> are two spellings of one item, so the
    /// <em>level</em> has to be decided before the spelling or an outer style's spelling beats an
    /// inner style's. See
    /// <see cref="OdfStyles.ResolveWithoutDefaults(string, OdfStyleFamily, OdfPropertyKind,
    /// IReadOnlyList{ValueTuple{string, string}}, out int)"/>.
    /// </remarks>
    private static readonly (string Namespace, string Name)[] FontSpellings =
    [
        (OdfNamespaces.FoCompatible, "font-family"),
        (OdfNamespaces.Style, "font-name"),
    ];

    /// <summary>
    /// Reads a shape's text, or null when it holds none.
    /// </summary>
    /// <param name="styles">The document's styles.</param>
    /// <param name="shape">The <c>draw:</c> element — a custom shape, a rectangle, a frame.</param>
    /// <returns>The text to draw, or null when the shape carries no paragraph with ink in it.</returns>
    public static SheetShapeText? Read(OdfStyles styles, XElement shape)
    {
        ArgumentNullException.ThrowIfNull(styles);
        ArgumentNullException.ThrowIfNull(shape);

        XElement container = TextContainer(shape);
        string? graphicStyle = Attribute(shape, OdfNamespaces.Draw, "style-name");
        SheetShapeAlignment? shapeAlignment = ShapeAlignment(styles, graphicStyle);

        List<SheetShapeParagraph> paragraphs = [];
        bool ink = false;

        foreach (XElement element in container.Elements(XName.Get("p", OdfNamespaces.Text)))
        {
            List<OdfStyleReference> cascade = [];
            if (Attribute(element, OdfNamespaces.Text, "style-name") is { } paragraphStyle)
                cascade.Add(new OdfStyleReference(paragraphStyle, OdfStyleFamily.Paragraph));

            List<SheetShapeRun> runs = [];
            Format trailing = FormatOf(styles, cascade);
            Append(styles, element, cascade, runs, ref trailing);

            foreach (SheetShapeRun run in runs)
            {
                if (run.Text.Length > 0) { ink = true; break; }
            }

            // A paragraph with nothing in it still occupies a line, and its height is its own
            // last span's rather than the shape's default: LibreOffice writes an empty line as
            // `<text:p><text:span text:style-name="T15"/></text:p>` and the EditEngine measures it
            // from the character attributes at the paragraph's own position
            // (`editeng/source/editeng/impedit3.cxx`:1896-1902). Carrying the trailing format as
            // an empty run is how the painter reads that height.
            if (runs.Count == 0)
                runs.Add(new SheetShapeRun(string.Empty, trailing.Size, trailing.Family, trailing.Bold));

            paragraphs.Add(new SheetShapeParagraph
            {
                Runs = runs,
                Alignment = shapeAlignment ?? ParagraphAlignment(styles, cascade),
            });
        }

        if (!ink) return null;

        return new SheetShapeText
        {
            Paragraphs = paragraphs,
            LeftInset = Padding(styles, graphicStyle, "padding-left"),
            RightInset = Padding(styles, graphicStyle, "padding-right"),
            TopInset = Padding(styles, graphicStyle, "padding-top"),
            BottomInset = Padding(styles, graphicStyle, "padding-bottom"),

            // `fo:wrap-option` is PROP_TextWordWrap (`xmloff/source/draw/sdpropls.cxx`:157), whose
            // pool default is on (`svdattr.cxx`:267).
            Wraps = !Graphic(styles, graphicStyle, OdfNamespaces.FoCompatible, "wrap-option")
                        .Is("no-wrap"),

            // `draw:textarea-vertical-align` is PROP_TextVerticalAdjust (`sdpropls.cxx`:140,
            // enumerated at `:658-665`), whose pool default is TOP (`include/svx/sdtaitm.hxx`:38).
            // `justify` is BLOCK — the block fills the box, so it starts at the top like TOP.
            Anchor = Graphic(styles, graphicStyle, OdfNamespaces.Draw, "textarea-vertical-align")
                         .Value switch
            {
                "middle" => SheetShapeAnchor.Middle,
                "bottom" => SheetShapeAnchor.Bottom,
                _ => SheetShapeAnchor.Top,
            },

            // ODF's spelling of DrawingML's `vertOverflow="clip"`, and the same UNO property:
            // `style:overflow-behavior` maps to PROP_TextClipVerticalOverflow (`sdpropls.cxx`:159)
            // through a named boolean whose true token is `clip` and whose false one is
            // `auto-create-new-frame` (`xmloff/source/style/prhdlfac.cxx`:483-487).
            ClipsVerticalOverflow =
                Graphic(styles, graphicStyle, OdfNamespaces.Style, "overflow-behavior").Is("clip"),

            Preset = OdsShapeInk.Preset(shape),
        };
    }

    /// <summary>The element whose <c>text:p</c> children are the shape's text.</summary>
    /// <remarks>
    /// A <c>draw:frame</c> wraps its text in a <c>draw:text-box</c>; every other shape carries the
    /// paragraphs directly. Both spellings are one text body once they reach the drawing layer.
    /// </remarks>
    private static XElement TextContainer(XElement shape)
        => shape.Name.LocalName == "frame"
           && shape.Element(XName.Get("text-box", OdfNamespaces.Draw)) is { } box
            ? box
            : shape;

    /// <summary>
    /// The alignment the shape imposes on every paragraph, or null when each decides for itself.
    /// </summary>
    /// <remarks>
    /// <c>draw:textarea-horizontal-align</c> is PROP_TextHorizontalAdjust
    /// (<c>xmloff/source/draw/sdpropls.cxx</c>:139, enumerated at <c>:649-656</c>), whose pool
    /// default is <c>SDRTEXTHORZADJUST_BLOCK</c> (<c>include/svx/sdtaitm.hxx</c>:64) — "use the
    /// whole text frame width" — under which each paragraph's own <c>fo:text-align</c> decides.
    /// The three explicit values override every paragraph at once: on the fixture,
    /// <c>center</c> moves all four of its lines from x 184.507 to 210.472 while a paragraph
    /// naming a centred style under <c>justify</c> moves that line alone, to 223.058.
    /// </remarks>
    private static SheetShapeAlignment? ShapeAlignment(OdfStyles styles, string? graphicStyle)
        => Graphic(styles, graphicStyle, OdfNamespaces.Draw, "textarea-horizontal-align").Value switch
        {
            "left" => SheetShapeAlignment.Left,
            "center" => SheetShapeAlignment.Centre,
            "right" => SheetShapeAlignment.Right,
            _ => null,
        };

    /// <summary>Where one paragraph sits across the box, from its own style chain.</summary>
    private static SheetShapeAlignment ParagraphAlignment(
        OdfStyles styles, IReadOnlyList<OdfStyleReference> cascade)
        => styles.ResolveProperty(
               cascade, OdfPropertyKind.Paragraph, OdfNamespaces.FoCompatible, "text-align")
               .Value switch
        {
            "center" => SheetShapeAlignment.Centre,
            "end" or "right" => SheetShapeAlignment.Right,
            _ => SheetShapeAlignment.Left,
        };

    /// <summary>One inset, or zero where no style in the chain states it.</summary>
    private static Length Padding(OdfStyles styles, string? graphicStyle, string name)
        => Graphic(styles, graphicStyle, OdfNamespaces.FoCompatible, name).AsLength() ?? Length.Zero;

    private static OdfProperty Graphic(
        OdfStyles styles, string? graphicStyle, string propertyNamespace, string name)
        => styles.ResolveProperty(
            graphicStyle, OdfStyleFamily.Graphic, OdfPropertyKind.Graphic, propertyNamespace, name);

    /// <summary>The size, face and weight one stretch of a paragraph is set in.</summary>
    private readonly record struct Format(Length Size, string? Family, bool Bold);

    /// <summary>
    /// Appends the runs one element's children contribute, following spans and hyperlinks.
    /// </summary>
    /// <remarks>
    /// Four node kinds carry characters and the corpus holds no other inside a sheet shape:
    /// text nodes, <c>text:span</c>, <c>text:s</c> — <c>text:c</c> repeated spaces — and
    /// <c>text:tab</c>. <c>text:a</c> is followed rather than skipped so a hyperlink's words are
    /// drawn; <c>text:line-break</c> cannot appear here without splitting the paragraph and is
    /// recorded in the module's TODO as unreached rather than implemented blind.
    /// </remarks>
    private static void Append(
        OdfStyles styles,
        XElement parent,
        List<OdfStyleReference> cascade,
        List<SheetShapeRun> runs,
        ref Format trailing)
    {
        foreach (XNode node in parent.Nodes())
        {
            if (node is XText text)
            {
                if (text.Value.Length > 0)
                    runs.Add(new SheetShapeRun(text.Value, trailing.Size, trailing.Family, trailing.Bold));
                continue;
            }

            if (node is not XElement element) continue;
            if (element.Name.NamespaceName != OdfNamespaces.Text) continue;

            switch (element.Name.LocalName)
            {
                case "span":
                {
                    int depth = cascade.Count;
                    if (Attribute(element, OdfNamespaces.Text, "style-name") is { } style)
                        cascade.Add(new OdfStyleReference(style, OdfStyleFamily.Text));

                    Format inner = FormatOf(styles, cascade);
                    Format nested = inner;
                    Append(styles, element, cascade, runs, ref nested);

                    cascade.RemoveRange(depth, cascade.Count - depth);

                    // The empty paragraph's height is the *last* span entered, not the largest and
                    // not the outermost, so the trailing format is taken from every span in turn.
                    trailing = inner;
                    break;
                }

                case "s":
                {
                    int count = Math.Clamp(
                        OdfValue.ParseInt(Attribute(element, OdfNamespaces.Text, "c")) ?? 1, 1, 4096);
                    runs.Add(new SheetShapeRun(
                        new string(' ', count), trailing.Size, trailing.Family, trailing.Bold));
                    break;
                }

                case "tab":
                    // Nothing is contributed, because a tab draws nothing and is not in the
                    // reference's text layer at all: read character by character out of
                    // 26.2.4.2's PDF of the fixture, `the` ends at 244.642 and `shape` begins at
                    // 245.735 with no glyph between them. What is dropped with it is that 1.09 pt
                    // of advance, which is a shape whose paragraph style states no tab stops
                    // taking the smallest step it can. **30 tabs in 5 of the 307 converted
                    // `.ods`**; a shape that does state tab stops is unreached, and both are in
                    // the module's TODO.
                    break;

                case "a":
                    Append(styles, element, cascade, runs, ref trailing);
                    break;

                default:
                    break;
            }
        }
    }

    /// <summary>The size, face and weight a cascade resolves to.</summary>
    private static Format FormatOf(OdfStyles styles, IReadOnlyList<OdfStyleReference> cascade)
        => new(
            SizeOf(styles, cascade),
            FamilyOf(styles, cascade),
            OdfTextFormat.IsBoldWeight(
                styles.ResolveProperty(
                    cascade, OdfPropertyKind.Text, OdfNamespaces.FoCompatible, "font-weight").Value));

    /// <summary>The em size a cascade resolves to.</summary>
    /// <remarks>
    /// <para>
    /// <strong>A percentage is passed over rather than applied.</strong> <c>fo:font-size</c> may
    /// be a proportion, and in ODF it is a proportion of the <em>parent style's</em> size rather
    /// than of the enclosing level's — a text style with no <c>style:parent-style-name</c> has
    /// nothing to take a proportion of, and that is what LibreOffice's own export writes. Measured
    /// on the fixture: a span stating <c>fo:font-size="50%"</c> inside a paragraph style stating
    /// 14 pt is drawn by 26.2.4.2 at <strong>14 pt</strong>, not at 7, so multiplying the outer
    /// size would halve it. Skipping the level and looking further out reproduces that exactly.
    /// </para>
    /// <para>
    /// No shape in the 307 converted <c>.ods</c> states one, so this is the safe reading of an
    /// unwitnessed case rather than a measured rule about proportions in general.
    /// </para>
    /// </remarks>
    private static Length SizeOf(OdfStyles styles, IReadOnlyList<OdfStyleReference> cascade)
    {
        for (int i = cascade.Count - 1; i >= 0; i--)
        {
            if (styles.ResolveWithoutDefaults(
                    cascade[i].Name, cascade[i].Family, OdfPropertyKind.Text,
                    OdfNamespaces.FoCompatible, "font-size").AsLength() is { } stated)
            {
                return stated;
            }
        }

        return DefaultSize;
    }

    /// <summary>
    /// The face a cascade asks for, deciding the level before the spelling.
    /// </summary>
    /// <remarks>
    /// The spellings are settled within one style's own parent chain and the chains are then tried
    /// innermost first, so a span stating <c>style:font-name</c> beats the shape's graphic style
    /// stating <c>fo:font-family</c> — which is the order LibreOffice's own export writes them in.
    /// A <c>style:font-name</c> names an <c>office:font-face-decls</c> entry rather than a family,
    /// and either spelling may hold a quoted CSS list.
    /// </remarks>
    private static string? FamilyOf(OdfStyles styles, IReadOnlyList<OdfStyleReference> cascade)
    {
        for (int i = cascade.Count - 1; i >= 0; i--)
        {
            OdfProperty found = styles.ResolveWithoutDefaults(
                cascade[i].Name, cascade[i].Family, OdfPropertyKind.Text, FontSpellings,
                out int matched);

            if (!found.HasValue) continue;

            string? family = matched == 1 ? FontFaceFamily(styles, found.Value) : found.Value;
            return FirstFamily(family);
        }

        return null;
    }

    private static string? FontFaceFamily(OdfStyles styles, string? name)
        => name is not null && styles.FontFaces.TryGetValue(name, out OdfFontFace? face)
            ? face.FontFamily ?? name
            : name;

    /// <summary>The first family of a CSS-style list, unquoted.</summary>
    private static string? FirstFamily(string? list)
    {
        if (list is not { Length: > 0 }) return null;

        int comma = list.IndexOf(',', StringComparison.Ordinal);
        string first = (comma >= 0 ? list[..comma] : list).Trim().Trim('\'', '"');
        return first.Length == 0 ? null : first;
    }

    private static string? Attribute(XElement element, string ns, string name)
        => element.Attribute(XName.Get(name, ns))?.Value;
}
