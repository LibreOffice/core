#FIXME move that to configure

# when you want to change the python version, you must update the d.lst
# in the python project accordingly !!!
PYMAJOR:=3
PYMINOR:=3
PYMICRO:=0
PYVERSION:=$(PYMAJOR).$(PYMINOR).$(PYMICRO)

ifeq ($(GUI),UNX)
ifeq ($(OS),MACOSX)
PY_FULL_DLL_NAME:=libpython$(PYMAJOR).$(PYMINOR).a
else
PY_FULL_DLL_NAME:=libpython$(PYMAJOR).$(PYMINOR)m.so
endif
else
ifeq ($(COM),GCC)
PY_FULL_DLL_NAME:=libpython$(PYMAJOR).$(PYMINOR).dll
else
PY_FULL_DLL_NAME:=python$(PYMAJOR)$(PYMINOR).dll
endif
endif
