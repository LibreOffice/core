# Pre-canned Distribution Configurations

These files contain sets of configure parameters used to build the
various Collabora Office variants. They can be passed on the
`autogen.sh` command line:

    ./autogen.sh --with-distro=CPLinux-LOKit

A line of the form `INCLUDE:OtherName` inlines the contents of
`OtherName.conf`. `Langs.conf` holds the shared `--with-lang=` list
included by the per-platform configs.
