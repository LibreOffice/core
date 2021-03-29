# Python UNO Bindings

UNO bindings for the Python programming language.

To have much joy debugging Python extensions you need to:

+ a) edit `pythonloader.py` in your install setting `DEBUG=1` at the top
+ b) `touch pyuno/source/module/pyuno_runtime.cxx` and `make debug=true` in `pyuno`

Then you'll start to see your exceptions on the console instead of them getting
lost at the UNO interface.

Python also comes with a gdb script
`libpython$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)m.so.1.0-gdb.py`
that is copied to `instdir` and will be auto-loaded by `gdb`;
it provides commands like `py-bt` to get a Python-level backtrace,
and `py-print` to print Python variables.

Another way to debug Python code is to use `pdb`: edit some initialization
function to insert `import pdb; pdb.set_trace()` (somewhere so that it is
executed early), then run `soffice` from a terminal and a command-line Python
debugger will appear where you can set Python-level breakpoints.
