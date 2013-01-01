# Set %%OOO_LIBRARY_PATH_VAR%% so that python.bin finds libpython2.6.so (this
# can go once python.bin contains a proper RPATH):
%%OOO_LIBRARY_PATH_VAR%%=$sd_prog${%%OOO_LIBRARY_PATH_VAR%%:+:$%%OOO_LIBRARY_PATH_VAR%%}
export %%OOO_LIBRARY_PATH_VAR%%

PYTHONPATH=$sd_prog:$sd_prog/python-core-%%PYVERSION%%/lib:$sd_prog/python-core-%%PYVERSION%%/lib/lib-dynload:$sd_prog/python-core-%%PYVERSION%%/lib/lib-tk:$sd_prog/python-core-%%PYVERSION%%/lib/site-packages${PYTHONPATH+:$PYTHONPATH}
export PYTHONPATH
PYTHONHOME=$sd_prog/python-core-%%PYVERSION%%
export PYTHONHOME

# execute binary
exec "$sd_prog/python.bin" "$@"
