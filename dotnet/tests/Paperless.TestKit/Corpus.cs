namespace Paperless.TestKit;

/// <summary>
/// Locates test-corpus files.
/// </summary>
/// <remarks>
/// The committed corpus lives in <c>dotnet/tests/corpus/</c>, so tests find it by walking up
/// from the test assembly rather than by relying on a copy-to-output step. That keeps the
/// files in one place instead of duplicating them into every test project's output.
/// </remarks>
public static class Corpus
{
    private static readonly Lazy<string?> RootPath = new(FindRoot);

    /// <summary>The corpus root directory, or null when it cannot be found.</summary>
    public static string? Root => RootPath.Value;

    /// <summary>True when the corpus is present.</summary>
    public static bool IsAvailable => RootPath.Value is not null;

    /// <summary>
    /// Resolves a corpus-relative path, searching the standard subdirectories.
    /// </summary>
    /// <returns>The full path, or null when the file is absent.</returns>
    public static string? Find(string relativePath)
    {
        if (RootPath.Value is null) return null;

        string direct = Path.Combine(RootPath.Value, relativePath);
        if (File.Exists(direct)) return direct;

        foreach (string bucket in (string[])["minimal", "features", "regression", "generated"])
        {
            string candidate = Path.Combine(RootPath.Value, bucket, relativePath);
            if (File.Exists(candidate)) return candidate;
        }
        return null;
    }

    /// <summary>
    /// Resolves a corpus file, failing the test with a useful message when it is missing.
    /// </summary>
    /// <remarks>
    /// A missing corpus file means the test cannot run at all, so this throws rather than
    /// returning null — silently skipping would let coverage quietly evaporate.
    /// </remarks>
    public static string Require(string relativePath)
    {
        string? found = Find(relativePath);
        if (found is not null) return found;

        throw new FileNotFoundException(
            $"Corpus file '{relativePath}' not found. Corpus root: {Root ?? "<not located>"}. "
            + "Generate the corpus with "
            + ".claude/skills/paperless-corpus/scripts/make-corpus.sh, or check that "
            + "dotnet/tests/corpus/ is present.",
            relativePath);
    }

    private static string? FindRoot()
    {
        // Walk up from the assembly location looking for tests/corpus. Ten levels is far
        // more than the bin/Debug/net10.0 nesting needs, and terminates on any layout.
        DirectoryInfo? dir = new(AppContext.BaseDirectory);
        for (int i = 0; i < 10 && dir is not null; i++, dir = dir.Parent)
        {
            string candidate = Path.Combine(dir.FullName, "tests", "corpus");
            if (Directory.Exists(candidate)) return candidate;

            // Also handle being run from inside tests/<project>/bin/...
            string sibling = Path.Combine(dir.FullName, "corpus");
            if (Directory.Exists(sibling) && File.Exists(Path.Combine(sibling, "README.md")))
                return sibling;
        }
        return null;
    }
}
