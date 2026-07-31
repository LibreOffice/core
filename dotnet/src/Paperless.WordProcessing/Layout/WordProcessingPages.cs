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

    internal WordProcessingPages(IReadOnlyList<LaidOutPage> pages)
    {
        ArgumentNullException.ThrowIfNull(pages);
        _pages = [.. pages.Select(page => new PageView(page))];
    }

    /// <inheritdoc/>
    public int Count => _pages.Count;

    /// <inheritdoc/>
    public IPage this[int index] => _pages[index];

    /// <summary>The laid-out pages, with the line boxes a renderer needs.</summary>
    public IReadOnlyList<LaidOutPage> Pages => [.. _pages.Select(page => page.Laid)];

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
