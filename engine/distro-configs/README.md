# Pre-canned Distribution Configurations

These files are supposed to correspond to the options used when
creating the Document Foundation (or other "canonical") builds of
LibreOffice for various platforms. They are *not* supposed to
represent the "most useful" options for developers in general. On the
contrary, the intent is that just running `./autogen.sh` without any
options at all should produce a buildable configuration for developers
with interest in working on the most commonly used parts of the code.

See <https://wiki.documentfoundation.org/Development/ReleaseBuilds> for how
TDF builds make use of these switches.  (Especially, since `--with-package-format`
now triggers whether or not installation sets are built, all the relevant `*.conf`
files specify it, except for `LibreOfficeLinux.conf`, where the TDF build
instructions pass an explicit `--with-package-format="rpm deb"` in addition to
`--with-distro=LibreOfficeLinux`.)

(Possibly the above is a misunderstanding, or maybe there never even
has been any clear consensus what situations these files actually are
intended for.)

The files contain sets of configuration parameters, and can be passed
on the `autogen.sh` command line thus:

    ./autogen.sh --with-distro=LibreOfficeFoo

Contrary to the above, in the Android case the amount of parameters
you just must use is so large, that for convenience it is always
easiest to use the corresponding distro-configs file. This is a bug
and needs to be fixed; also configuring for Android should ideally use
sane (or the only possible) defaults and work fine without any
parameters at all.
