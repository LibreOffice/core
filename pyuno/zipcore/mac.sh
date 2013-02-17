PYTHONHOME=$sd_prog/../Frameworks/LibreOfficePython.framework
export PYTHONHOME

pybasislibdir=$PYTHONHOME/Versions/%%PYVERSION%%/lib/python%%PYVERSION%%
PYTHONPATH=$sd_prog:$pybasislibdir:$pybasislibdir/lib-dynload:$pybasislibdir/lib-tk:$pybasislibdir/site-packages${PYTHONPATH+:$PYTHONPATH}
export PYTHONPATH

# execute binary
exec "$PYTHONHOME/Versions/%%PYVERSION%%/Resources/Python.app/Contents/MacOS/LibreOfficePython" "$@"
