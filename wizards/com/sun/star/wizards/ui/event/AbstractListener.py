from MethodInvocation import MethodInvocation
import traceback
'''
This class is a base class for listener classes.
It uses a hashtable to map between a ComponentName, EventName and a MethodInvokation Object.
To use this class do the following:<br/>
<list>
<li>Write a subclass which implements the needed Listener(s).</li>
in the even methods, use invoke(...).
<li>When instanciating the component, register the subclass as the event listener.</li>
<li>Write the methods which should be performed when the event occures.</li>
<li>call the "add" method, to define a component-event-action mapping.</li>
</list>
@author  rpiterman
'''

class AbstractListener(object):
    '''Creates a new instance of AbstractListener'''

    mHashtable = {}

    def add(self, componentName, eventName, mi, target=None):
        try:
            if target is not None:
                mi =  MethodInvocation(mi, target)
            AbstractListener.mHashtable[componentName + eventName] = mi
        except Exception, e:
            traceback.print_exc()


    def get(self, componentName, eventName):
        return AbstractListener.mHashtable[componentName + eventName]

    @classmethod
    def invoke(self, componentName, eventName, param):
        try:
            mi = self.get(componentName, eventName)
            if mi != None:
                return mi.invoke(param)
            else:
                return None

        except InvocationTargetException, ite:
            print "======================================================="
            print "=== Note: An Exception was thrown which should have ==="
            print "=== caused a crash. I caught it. Please report this ==="
            print "=== to  libreoffice.org                             ==="
            print "======================================================="
            traceback.print_exc()
        except IllegalAccessException, iae:
            traceback.print_exc()
        except Exception, ex:
            print "======================================================="
            print "=== Note: An Exception was thrown which should have ==="
            print "=== caused a crash. I Catched it. Please report this =="
            print "=== to  libreoffice.org                              =="
            print "======================================================="
            traceback.print_exc()

        return None

    '''
    Rerurns the property "name" of the Object which is the source of the event.
    '''
    def getEventSourceName(self, eventObject):
        return Helper.getUnoPropertyValue(eventObject.Source.getModel(), PropertyNames.PROPERTY_NAME)

