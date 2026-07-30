using System.Xml.Linq;
using Paperless.Core.Extraction;
using Paperless.Core.Formats;
using Paperless.OpenDocument;

namespace Paperless.Presentations.OpenDocument;

/// <summary>
/// Reads ODF presentations: <c>odp</c>, the <c>otp</c> template, and the flat <c>fodp</c>.
/// </summary>
/// <remarks>
/// <para>
/// Each <c>draw:page</c> becomes a <see cref="SectionKind.Slide"/> section, with its speaker
/// notes following as a <see cref="SectionKind.SlideNotes"/> section so the two stay adjacent
/// and separately identifiable.
/// </para>
/// <para>
/// Shapes are read in document order rather than sorted by position. Document order is the
/// order the file states, and it is what the authoring application used; inferring a reading
/// order from coordinates would guess, and guess differently for every layout.
/// </para>
/// </remarks>
public sealed class OdpReader : OdfReader
{
    /// <inheritdoc/>
    protected override DocumentFamily Family => DocumentFamily.Presentation;

    /// <inheritdoc/>
    protected override void ReadBody(XElement body, OdfContentReader reader, ContentDocument content)
    {
        ArgumentNullException.ThrowIfNull(body);
        ArgumentNullException.ThrowIfNull(reader);
        ArgumentNullException.ThrowIfNull(content);

        int index = 0;
        foreach (XElement page in body.Elements(XName.Get("page", OdfNamespaces.Draw)))
        {
            ContentSection slide = new()
            {
                Kind = SectionKind.Slide,
                Index = index,
                Name = OdfContentReader.Attribute(page, OdfNamespaces.Draw, "name"),
                IsHidden = reader.IsDrawingPageHidden(page),
            };
            reader.ReadBlocks(page, slide);
            content.Children.Add(slide);

            // presentation:notes is a sibling of the slide's shapes inside the same page, so
            // the walk above deliberately skips it and it is read here instead — otherwise the
            // notes' text would be indistinguishable from the slide's own.
            XElement? notes = page.Element(XName.Get("notes", OdfNamespaces.Presentation));
            if (notes is not null)
            {
                ContentSection notesSection = new()
                {
                    Kind = SectionKind.SlideNotes,
                    Index = index,
                    Name = slide.Name,
                };
                reader.ReadBlocks(notes, notesSection);

                // A notes page always exists in a LibreOffice-written deck, carrying a slide
                // thumbnail and an empty placeholder. Only add it when it holds actual text.
                if (notesSection.GetText().Trim().Length > 0) content.Children.Add(notesSection);
            }

            foreach (ContentNode node in reader.TakeHoisted()) content.Children.Add(node);
            index++;
        }
    }
}
