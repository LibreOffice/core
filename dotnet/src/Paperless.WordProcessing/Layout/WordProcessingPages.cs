using Paperless.Core.Documents;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;

namespace Paperless.WordProcessing.Layout;

/// <summary>
/// The pages a word-processing document laid out to.
/// </summary>
/// <remarks>
/// The adapter between the paginator's own result and the family-neutral
/// <see cref="IPageSequence"/> a caller asks for. The two are kept apart because the paginator's page
/// carries the line boxes a renderer needs and a spreadsheet's or a slide's does not — there is nothing
/// useful in an abstraction that covers all three beyond a count and a size.
/// </remarks>
public sealed class WordProcessingPages : IPageSequence
{
    private readonly List<PageView> _pages;

    internal WordProcessingPages(
        IReadOnlyList<LaidOutPage> pages, IReadOnlyList<PageParagraph>? paragraphs = null)
    {
        ArgumentNullException.ThrowIfNull(pages);
        _pages = [.. pages.Select(page => new PageView(page))];
        Paragraphs = paragraphs ?? [];
    }

    /// <inheritdoc/>
    public int Count => _pages.Count;

    /// <inheritdoc/>
    public IPage this[int index] => _pages[index];

    /// <summary>The laid-out pages, with the line boxes a renderer needs.</summary>
    public IReadOnlyList<LaidOutPage> Pages => [.. _pages.Select(page => page.Laid)];

    /// <summary>
    /// The paragraphs the pages were filled from, indexed by <see cref="PlacedLine.ParagraphIndex"/>.
    /// </summary>
    /// <remarks>
    /// Carried rather than left to the caller to rebuild, because a <see cref="PlacedLine"/> holds a
    /// character range and not the characters: pagination places lines and leaves the strings where they
    /// were. Anything reading a page back — a renderer, or a test comparing words — needs both halves,
    /// and rebuilding the list means re-resolving every style and re-reading every font.
    /// </remarks>
    public IReadOnlyList<PageParagraph> Paragraphs { get; }

    /// <summary>
    /// The text of a placed line.
    /// </summary>
    /// <remarks>
    /// Empty when the line's paragraph is out of range, which happens only if a caller built this without
    /// its paragraphs — worth answering rather than throwing, since a page whose text cannot be recovered
    /// is still a page with a size and a body area.
    /// </remarks>
    public string TextOf(PlacedLine line)
    {
        if (line.ParagraphIndex < 0 || line.ParagraphIndex >= Paragraphs.Count) return string.Empty;

        string paragraph = Paragraphs[line.ParagraphIndex].Text;
        return line.Box.Line.VisibleTextIn(paragraph).ToString();
    }

    private sealed class PageView(LaidOutPage laid) : IPage
    {
        internal LaidOutPage Laid { get; } = laid;

        /// <inheritdoc/>
        public int Index => Laid.Index;

        /// <inheritdoc/>
        public DocSize Size => Laid.Size;

        /// <inheritdoc/>
        /// <remarks>
        /// The printed page number, which is not the index: a section can restart numbering, so a title
        /// page may be numbered zero and the page after it one.
        /// </remarks>
        public string? Label => Laid.Number.ToString(System.Globalization.CultureInfo.InvariantCulture);

        /// <inheritdoc/>
        /// <remarks>
        /// Not implemented yet. Drawing needs the glyph runs, and those need each line's characters
        /// shaped again with the runs' own formatting rather than the paragraph's — the pass that
        /// resolves run formatting. Throwing says so rather than silently producing a blank page, which
        /// would look like a rendering bug rather than a missing feature.
        /// </remarks>
        public void Draw(IDrawingSink sink)
        {
            ArgumentNullException.ThrowIfNull(sink);
            throw new NotSupportedException(
                "Drawing a laid-out page is not implemented yet; the line boxes are available through "
                + "WordProcessingPages.Pages.");
        }
    }
}
