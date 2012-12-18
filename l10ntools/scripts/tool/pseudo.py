#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************

# to support macosx baseline machines from Cretaceous period

# incomplete set() class implementation of Python 2.4
class PseudoSet:
    _list = []

    def __str__(self):
        return str(self._list)

    def __init__(self, newlist=[]):
        self._list = self._remove_dupes(newlist)

    def __or__(self, other):
        tmplist = []
        if self._list != None and other != None:
            tmplist.extend(self._list)
            tmplist.extend(other)
            return PseudoSet(self._remove_dupes(tmplist))
        else:
            print("__or__(None)")

    def __sub__(self,other):
        tmplist = []
        if self._list != None and other != None:
            tmplist.extend(self._list)
            [tmplist.remove(key) for key in other if key in tmplist]
        else:
            print("__sub__(none)")
        return PseudoSet(tmplist)

    def __and__(self, other):
        tmplist = []
        if other != None and self._list != None:
            [tmplist.append(key) for key in self._list if key in other]
            return PseudoSet(tmplist)
        else:
            print("__and__(None)")

    def __iter__(self):
        return self._list.__iter__()

    def __items__(self):
        return list(self._list.items())

    def __keys__(self):
        return keys(self._list)

    def _remove_dupes(self, list):
        tmpdict = {}
        for key in list:
            tmpdict[key] = 1
        return list(tmpdict.keys())

# incomplete OrderedDict() class implementation
class PseudoOrderedDict(dict):
    _keylist        = []
    _valuelist      = []

    def __init__(self, defaults={}):
        dict.__init__(self)
        for n,v in list(defaults.items()):
            self[n] = v

    def __setitem__(self, key, value):
        self._keylist.append(key)
        self._valuelist.append(value)
        return dict.__setitem__(self, key, value)

    def __delattr__(self, key):
        self._keylist.__delattr__(key)
        self._valuelist.__delattr__(dict[key])
        return dict.__delattr__(self, key)

    def __delitem__(self, key):
        self._keylist.__delitem__(key)
        self._valuelist.__delitem__(dict[key])
        return dict.__delitem__(self, key)

    def __iter__(self):
        raise NotImplementedError("__iter__")

    def __iterkeys__(self):
        return self._keylist

    def iteritems(self):
        #return self._valuelist
        return list(zip(self._keylist, self._valuelist))

    def items(self):
        return list(zip(self._keylist,self._valuelist))

    def __keys__(self):
        return self._keylist

    def keys(self):
        return self._keylist

    def __keysattr__(self):
        return self._keylist

    def pop(self, key):
        self._keylist.pop(key)
        self._valuelist.pop(key)
        return dict.__pop__(self, key)

    def popitem(self):
        raise NotImplementedError("popitem")

def _testdriver_set():
    list, list1 = [] ,[]
    list.append("a")
    list.append("b")
    list.append("c")

    list1.append("a")
    list1.append("b")
    list1.append("d")
    list1.append("e")
    list1.append("e")

    if "a" in list:
        print("YEAH!")

    a = PseudoSet(list)
    b = PseudoSet(list1)

    print("a="+str(a))
    print("b="+str(b))
    print("a|b=" + str(a|b))
    print("a="+str(a))
    print("b="+str(b))
    print("a&b=" + str(a&b))
    print("a="+str(a))
    print("b="+str(b))
    print("a-b" + str(a-b))

    for key in a:
        print(key)

def _testdriver_dict():
    d = PseudoOrderedDict()
    d["a"] = 1
    d["b"] = 2
    d["c"] = 3
    d["d"] = 4
    d["e"] = 5
    d["f"] = 6

    print("a="+str(d["a"]))
    print("e="+str(d["e"]))
    for key,value in d.items():
        print("d["+key+"]="+str(d[key]))
        print("key="+str(key)+" value="+str(value))

    print("keys="+str(list(d.keys())))

#_testdriver_dict()
