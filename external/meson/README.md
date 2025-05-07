Meson build tool

https://github.com/mesonbuild/meson/

using internal copy to ensure it can be called with fully-internal python.
Some distro installed versions of meson cannot be called with different python
runtimes and when not using fully-internal python the whole point of
fully-internal python is invalidated…
