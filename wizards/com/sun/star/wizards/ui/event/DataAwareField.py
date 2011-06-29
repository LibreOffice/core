import traceback
from DataAware import *
import uno

class DataAwareField(object):

    def __init__(self, field, convertTo):
        self.convertTo = convertTo
        self.field = field

    def get(self, target):
        try:
            i = getattr(target, self.field)
            if isinstance(self.convertTo, bool):
                if i:
                    return True
                else:
                    return False
            elif isinstance(self.convertTo,int):
                return int(i)
            elif isinstance(self.convertTo,str):
                return str(i)
            elif self.convertTo.type == uno.Any("short",0).type:
                return uno.Any("[]short",(i,))
            else:
                raise AttributeError(
                    "Cannot convert int value to given type (" + \
                    str(type(self.convertTo)) + ").")

        except AttributeError, ex:
            traceback.print_exc()
            return None

    def Set(self, value, target):
            setattr(target, self.field, value)
