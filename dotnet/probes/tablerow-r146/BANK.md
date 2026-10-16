# The reference bank this round built, and where it went

Rendering the whole words track through 26.2.4.2 produced **337 of 337** PDFs, 121 MB, and they
are worth keeping: this project has repeatedly wanted a reference bank and has repeatedly had to
re-render one (`dotnet/CLAUDE.md`'s *"there is nothing to reuse"*). They are **not** committed —
git keeps every version of every binary, and 121 MB of regenerable output is exactly what the
corpus rules say not to put in the tree.

They live at **`/home/user/refpdfs-words-26.2.4.2/`**, in the three worker directories
`0`, `1`, `2` the sweep wrote, one PDF per document named after its stem.

**Check before reusing them.** They are a *container-local* bank with no manifest of the binary
or the font set, which is the failure mode `dotnet/probes/PROVENANCE.tsv` exists to record: they
were taken on 2026-09-17 against `/opt/libreoffice26.2/program/soffice` 26.2.4.2
(`0229ac93fcf0d7cbc6376066c6f35021cef002dc`) with the tarball's duplicate, Latin Noto, Narrow and
Condensed faces moved aside. A container restart destroys them, and any of the 49 documents
naming a Narrow family needs re-rendering if the font set moves again.

`sweep-ref.sh` is what built them; it takes about forty minutes at three workers.
