using System.Globalization;
using System.Xml.Linq;
using Paperless.Ooxml.DrawingML;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// Builds the <c>p:txBody</c> an evaluated node draws, from the paragraphs it gathered and the
/// decisions the <c>tx</c> algorithm made about them.
/// </summary>
/// <remarks>
/// <para>
/// The paragraphs themselves are the author's, copied out of the data model unchanged — they are
/// ordinary DrawingML and the slide's own text reader reads them. What this adds is everything
/// the layout definition decided rather than the author: the outline level, the bullet, the
/// alignment, the anchor and the insets.
/// </para>
/// <para>
/// <strong>The bullet is not the author's either.</strong> A diagram's sub-levels get a solid
/// round bullet whatever the text list style says, because the layout definition has no way to
/// state one and PowerPoint draws one anyway — LibreOffice writes "It is not possible to change
/// the bullet style for text" and hard-codes <c>•</c> (<c>diagramlayoutatoms.cxx:1820</c>).
/// The indent is hard-coded with it: 285 750 EMU per level, which is a quarter inch, hanging.
/// </para>
/// </remarks>
internal static class PptxDiagramText
{
    /// <summary>The indent one bullet level costs, in EMUs — a quarter of an inch.</summary>
    private const int BulletIndent = 285750;

    /// <summary>Builds the text body.</summary>
    public static XElement Body(DiagramShape shape)
    {
        XElement body = new(Ppt.Name("txBody"), BodyProperties(shape));

        XElement? listStyle = Drawing.Child(shape.TextBodySource, "lstStyle");
        body.Add(listStyle is not null ? new XElement(listStyle) : new XElement(Drawing.Name("lstStyle")));

        foreach (DiagramTextParagraph paragraph in shape.Text)
        {
            body.Add(Paragraph(shape, paragraph));
        }

        return body;
    }

    private static XElement BodyProperties(DiagramShape shape)
    {
        XElement? source = Drawing.Child(shape.TextBodySource, "bodyPr");
        XElement properties = source is not null
            ? new XElement(source)
            : new XElement(Drawing.Name("bodyPr"));

        properties.SetAttributeValue(
            "anchor",
            shape.TextAnchor switch { "t" => "t", "b" => "b", _ => "ctr" });

        if (shape.TextRotation != 0)
        {
            properties.SetAttributeValue("rot", Number(shape.TextRotation));
        }

        if (shape.HasTextInsets)
        {
            properties.SetAttributeValue("lIns", Number(shape.TextInsets.Left));
            properties.SetAttributeValue("tIns", Number(shape.TextInsets.Top));
            properties.SetAttributeValue("rIns", Number(shape.TextInsets.Right));
            properties.SetAttributeValue("bIns", Number(shape.TextInsets.Bottom));
        }

        if (shape.AutoFitText)
        {
            // EG_TextAutofit is a choice, so whatever the author's body asked for goes: the tx
            // algorithm sets the property outright rather than merging with it.
            properties.Elements(Drawing.Name("noAutofit")).Remove();
            properties.Elements(Drawing.Name("spAutoFit")).Remove();
            properties.Elements(Drawing.Name("normAutofit")).Remove();
            properties.Add(new XElement(Drawing.Name("normAutofit")));
        }

        return properties;
    }

    private static XElement Paragraph(DiagramShape shape, DiagramTextParagraph source)
    {
        XElement paragraph = new(source.Paragraph);

        XElement? properties = Drawing.Child(paragraph, "pPr");
        if (properties is null)
        {
            properties = new XElement(Drawing.Name("pPr"));
            paragraph.AddFirst(properties);
        }

        if (source.Level > 0)
        {
            properties.SetAttributeValue("lvl", Number(source.Level));
        }

        if (shape.TextAlignment is { } alignment) properties.SetAttributeValue("algn", alignment);

        if (source.Level >= shape.BulletLevel)
        {
            if (Drawing.Attribute(properties, "marL") is null)
            {
                properties.SetAttributeValue(
                    "marL", Number(BulletIndent * (source.Level - shape.BulletLevel + 1)));
            }

            if (Drawing.Attribute(properties, "indent") is null)
            {
                properties.SetAttributeValue("indent", Number(-BulletIndent));
            }

            properties.Elements(Drawing.Name("buNone")).Remove();
            properties.Elements(Drawing.Name("buChar")).Remove();
            properties.Elements(Drawing.Name("buAutoNum")).Remove();
            properties.Add(new XElement(Drawing.Name("buChar"), new XAttribute("char", "•")));
        }
        else
        {
            // Above the bullet level a diagram's line is a plain line, and the master's list
            // style would otherwise put a bullet on it.
            properties.Elements(Drawing.Name("buChar")).Remove();
            properties.Elements(Drawing.Name("buAutoNum")).Remove();
            properties.Add(new XElement(Drawing.Name("buNone")));
        }

        if (shape.FontSize > 0) ApplyFontSize(paragraph, shape.FontSize);

        return paragraph;
    }

    /// <summary>
    /// Gives every run that states no size the one the <c>primFontSz</c> constraint asked for.
    /// </summary>
    /// <remarks>
    /// A run that states its own size keeps it, which is how a node the author resized stays
    /// resized. <c>diagramlayoutatoms.cxx:1715-1721</c>.
    /// </remarks>
    private static void ApplyFontSize(XElement paragraph, double points)
    {
        string hundredths = ((int)Math.Round(points * 100)).ToString(CultureInfo.InvariantCulture);

        foreach (XElement run in paragraph.Elements(Drawing.Name("r")))
        {
            XElement? properties = Drawing.Child(run, "rPr");
            if (properties is null)
            {
                properties = new XElement(Drawing.Name("rPr"));
                run.AddFirst(properties);
            }

            if (Drawing.Attribute(properties, "sz") is null)
            {
                properties.SetAttributeValue("sz", hundredths);
            }
        }
    }

    private static string Number(int value) => value.ToString(CultureInfo.InvariantCulture);
}
