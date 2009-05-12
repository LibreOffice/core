There seems to be some problems with load test libraries from the right path,
if the libraries are stored also in the solver directory.

To test this:
Call dmake to build the libraries in this selftest directory.
Call deliver to copy one test library into the solver tree.
Call dmake test to simply test the behaviour.

Some more words:

Per default the test libraries should not deliver but if:

- Linux and Windows will use an absolute path to load the test libraries
  by dlopen().
- MacOSX will use local path per default to load the test libraries by
  dlopen();

If there are problems, there exists 2 new parameters like
-absolutepath
-localpath

Just call
'dmake test TESTOPTADD="-absolutepath"' or
'dmake test TESTOPTADD="-localpath"' from this directory and see which
behaviour is the right for your environment.

There exist some environment variables to manipulate the default behaviour
of load libraries. These are
LD_LIBRARY_PATH
DYLD_LIBRARY_PATH (Mac OS X)
