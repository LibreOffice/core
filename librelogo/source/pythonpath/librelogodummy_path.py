import uno, re, sys, os, traceback
def get_path():
    return os.path.join(os.path.dirname(sys.modules[__name__].__file__), __name__ + ".py")
