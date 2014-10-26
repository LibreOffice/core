# Set URE_BOOTSTRAP so that "uno.getComponentContext()" bootstraps a complete
# OOo UNO environment:
: ${URE_BOOTSTRAP=vnd.sun.star.pathname:$sd_prog/%%PYTHON_FUNDAMENTALRC_RELATIVE_PATH%%/fundamentalrc}
export URE_BOOTSTRAP

PYTHONHOME=$sd_prog/%%PYTHON_FRAMEWORK_RELATIVE_PATH%%/LibreOfficePython.framework
export PYTHONHOME

pybasislibdir=$PYTHONHOME/Versions/%%PYVERSION%%/lib/python%%PYVERSION%%
PYTHONPATH=$sd_prog/%%PYTHON_FUNDAMENTALRC_RELATIVE_PATH%%:$sd_prog/%%PYTHON_FRAMEWORK_RELATIVE_PATH%%:$pybasislibdir:$pybasislibdir/lib-dynload:$pybasislibdir/lib-tk:$pybasislibdir/site-packages${PYTHONPATH+:$PYTHONPATH}
export PYTHONPATH

# execute binary
exec "$PYTHONHOME/Versions/%%PYVERSION%%/Resources/Python.app/Contents/MacOS/LibreOfficePython" "$@"
