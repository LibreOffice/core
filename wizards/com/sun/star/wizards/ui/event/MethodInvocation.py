'''import java.lang.reflect.InvocationTargetException;

import java.lang.reflect.Method;

import com.sun.star.wizards.common.PropertyNames;
'''

'''Encapsulate a Method invocation.
In the constructor one defines a method, a target object and an optional
Parameter.
Then one calls "invoke", with or without a parameter. <br/>
Limitations: I do not check anything myself. If the param is not ok, from the
wrong type, or the mothod doesnot exist on the given object.
You can trick this class howmuch you want: it will all throw exceptions
on the java level. i throw no error warnings or my own excceptions...
'''

class MethodInvocation(object):

    EMPTY_ARRAY = ()

    '''Creates a new instance of MethodInvokation'''
    def __init__(self, method, obj, paramClass=None):
        self.mMethod = method
        self.mObject = obj
        self.mWithParam = not (paramClass==None)

    '''Returns the result of calling the method on the object, or null,
    if no result. '''

    def invoke(self, param=None):
        if self.mWithParam:
            return self.mMethod.invoke(self.mObject, (param))
        else:
            return self.mMethod.invoke(self.mObject, EMPTY_ARRAY)
