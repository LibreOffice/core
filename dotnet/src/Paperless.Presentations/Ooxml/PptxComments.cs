using System.Xml.Linq;
using Paperless.Core.Extraction;

namespace Paperless.Presentations.Ooxml;

/// <summary>
/// Reads a slide's review comments and the deck's list of comment authors.
/// </summary>
/// <remarks>
/// <para>
/// A comment is a part of its own, reached by a <c>comments</c> relationship <em>from the
/// slide</em>; the authors are a second part reached by a <c>commentAuthors</c> relationship
/// from the presentation. Neither is named in the slide's XML, and neither can be found by
/// convention with any confidence — LibreOffice hard-codes <c>ppt/commentAuthors.xml</c>
/// (<c>oox/source/ppt/presentationfragmenthandler.cxx:666</c>) and that name is only a
/// convention, so the relationship is tried first and the conventional name kept as a fallback.
/// </para>
/// <para>
/// The text is a single <c>p:text</c> element, not a DrawingML text body: a comment has no runs
/// and no formatting, and a multi-paragraph comment arrives as one string with newlines in it.
/// That is what LibreOffice writes for a two-paragraph Impress annotation, and what it reads
/// back through <c>onCharacters</c> (<c>oox/source/ppt/slidefragmenthandler.cxx:219-228</c>).
/// </para>
/// <para>
/// Not read: the "modern" comments PowerPoint 365 writes into a separate
/// <c>ppt/comments/modernComment_*.xml</c> part under an <c>p188:</c> vocabulary. LibreOffice
/// 24.2 does not read those either, and a deck carrying them also carries the legacy
/// <c>p:cmLst</c> for compatibility, so nothing is lost on the files that exist today.
/// </para>
/// </remarks>
internal static class PptxComments
{
    /// <summary>The conventional author-list part name, used only when no relationship names one.</summary>
    private const string ConventionalAuthorsPart = "ppt/commentAuthors.xml";

    /// <summary>
    /// The comments on one slide, in the order the part states them.
    /// </summary>
    /// <param name="file">The open deck.</param>
    /// <param name="slide">The slide whose comments are wanted.</param>
    public static IEnumerable<ContentSection> Read(PptxFile file, PptxSlide slide)
    {
        XElement? root = file.Load(file.TargetOfType(slide.PartName, "comments"));
        if (root is null) yield break;

        IReadOnlyDictionary<string, string> authors = file.CommentAuthors;

        int index = 0;
        foreach (XElement comment in Ppt.Children(root, "cm"))
        {
            string text = Ppt.Child(comment, "text")?.Value ?? string.Empty;

            ContentSection section = new()
            {
                Kind = SectionKind.Comment,
                Index = index++,

                // The author is an id into the deck-wide list. An id naming nobody leaves the
                // name null rather than inventing one; LibreOffice substitutes "Anonymous"
                // (oox/source/ppt/comments.cxx:70), which would claim the file says something
                // it does not.
                Name = Ppt.Attribute(comment, "authorId") is { } id
                       && authors.TryGetValue(id, out string? author)
                    ? author
                    : null,
            };

            // A comment carries no runs, so its paragraphs are its newlines. An empty comment
            // is still a comment somebody left, and its author is worth reporting.
            foreach (string line in text.Split('\n'))
            {
                ContentParagraph paragraph = new();
                string trimmed = line.TrimEnd('\r');
                if (trimmed.Length > 0) paragraph.Children.Add(new ContentRun { Text = trimmed });
                section.Children.Add(paragraph);
            }

            yield return section;
        }
    }

    /// <summary>
    /// The deck's comment authors, by the id a comment refers to them with.
    /// </summary>
    /// <param name="file">The open deck.</param>
    public static Dictionary<string, string> ReadAuthors(PptxFile file)
    {
        Dictionary<string, string> authors = new(StringComparer.Ordinal);

        XElement? root = file.Load(file.TargetOfType(file.MainPartName, "commentAuthors"))
                         ?? file.Load(ConventionalAuthorsPart);
        if (root is null) return authors;

        foreach (XElement author in Ppt.Children(root, "cmAuthor"))
        {
            if (Ppt.Attribute(author, "id") is not { } id) continue;
            authors[id] = Ppt.Attribute(author, "name") ?? string.Empty;
        }

        return authors;
    }
}
