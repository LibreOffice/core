# Tools and Non-Build Scripts

Tools and scripts mostly not used during the build

This direction has a number of key pieces (?) that are used during the
build, or are simply generally useful. One example is `bin/run`.

This utility can be used to run the executables in this folder:

    workdir/LinkTarget/Executable

Format of the usage is:

    ./bin/run application [parameters]|--list|--help

Use --list (same as -list or -l) to get the list of executables
Use --help (same as -help or -h) to get this help

Another example is:

    bin/find-german-comments <directory>

which will try to detect and extract all the German comments in a
given source code hierarchy `/` directory.

Please note that the associated issue [tdf#39468](https://bugs.documentfoundation.org/show_bug.cgi?id=39468) is now marked as
RESOLVED/FIXED, so don't expect many German comments.

