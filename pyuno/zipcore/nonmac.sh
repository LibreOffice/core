
PYTHONPATH=$sd_prog:$sd_prog/python-core-%%PYVERSION%%/lib:$sd_prog/python-core-%%PYVERSION%%/lib/lib-dynload:$sd_prog/python-core-%%PYVERSION%%/lib/lib-tk:$sd_prog/python-core-%%PYVERSION%%/lib/site-packages${PYTHONPATH+:$PYTHONPATH}
export PYTHONPATH
PYTHONHOME=$sd_prog/python-core-%%PYVERSION%%
export PYTHONHOME

# execute binary
exec "$sd_prog/python.bin" "$@"
