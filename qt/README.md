Build instruction have moved to https://www.collaboraonline.org/post/build-co-linux/

# Release helpers

`tools/add-release.py` is a script that allow adding a release to the
appstream file prior to tagging.

Usage

```shell
$ ./tools/add-release.py 25.04.7.8.1 com.collaboraoffice.Office.metainfo.xml
```

This will add release `25.04.7.8.1` with today's date in the appstream
file `com.collaboraoffice.Office.metainfo.xml`

Options are:

- `-d` or `--date` to pass the date string to use.
- `-c` or `--changelog` to use the changelog entry from a file.

The expected changelog data should follow appstream `description`
element.

```xml
<description>
<p>Foo</p>
<ul>
<li>Something</li>
</ul>
</description>
```

# Debugging

Some useful environment variable settings are `QT_LOGGING_RULES=` to always get JS console warnings
and errors (even when running without a `--debug` command line argument) and
`QT_MESSAGE_PATTERN=%{if-category}%{category}: %{endif}%{file}:%{line}: %{message}` to get JS
console warnings and errors that contain file and line information.
