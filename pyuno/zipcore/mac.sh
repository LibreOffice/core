# Set URE_BOOTSTRAP so that "uno.getComponentContext()" bootstraps a complete
# OOo UNO environment:
: ${URE_BOOTSTRAP=vnd.sun.star.pathname:$sd_prog/../Resources/fundamentalrc}
export URE_BOOTSTRAP

PYTHONHOME=$sd_prog/../Frameworks/LibreOfficePython.framework
export PYTHONHOME

pybasislibdir=$PYTHONHOME/Versions/%%PYVERSION%%/lib/python%%PYVERSION%%
PYTHONPATH=$sd_prog/../Resources:$sd_prog/../Frameworks:$pybasislibdir:$pybasislibdir/lib-dynload:$pybasislibdir/site-packages${PYTHONPATH+:$PYTHONPATH}
export PYTHONPATH

# execute binary
exec "$PYTHONHOME/Versions/%%PYVERSION%%/Resources/Python.app/Contents/MacOS/LibreOfficePython" "$@"
