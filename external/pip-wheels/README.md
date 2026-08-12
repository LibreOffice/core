Python PIP wheels required for build time tools

CPython can use them directly so no need to build anything.

Downloaded with:

  pip download --only-binary=:all: -d . dnfile macholib pyelftools

Currently contains:
* pyelftools (Public Domain)
* pefile, dnfile (MIT license)
* macholib, altgraph (MIT license)

