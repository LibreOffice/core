# to activate run the command below when inside lldb
#   command script import /tools/lldb4aoo.py
# or add the line to ~/.lldbinit to always activate it

def __lldb_init_module( dbg, dict):
	# the list of AOO specific types
	aoo_types = ['rtl_String', 'rtl::OString', 'rtl_uString', 'rtl::OUString',
		    '_ByteStringData', '_UniStringData', 'ByteString', 'UniString']
	# register a helper function for each type
	for t in aoo_types:
		f = 'getinfo_for_' + t.replace( '::', '_')
		if f in globals():
			dbg.HandleCommand( 'type summary add %s -F %s.%s' % (t,__name__,f))
		else:
			print( 'AOO-LLDB helper function "%s" is not yet defined: "%s" types cannot be displayed properly!' % (f,t))

	# perform some goodies if the process is ready to run or already running
	if dbg.GetNumTargets() > 0:
		# the list of interesting function breakpoints
		aoo_breakfn = ['main', '__cxa_call_unexpected', 'objc_exception_throw']
		aoo_breakfn += ['__cxa_throw']
		# register the function breakpoints
		for t in aoo_breakfn:
			dbg.HandleCommand( 'breakpoint set -n ' + t)


# definitions for individual LLDB type summary helpers 

def ret_strinfo( refs, length, ary0):
	a = ary0.AddressOf().GetPointeeData( 0, length)
	if ary0.GetByteSize() == 1:
		s = ''.join([chr(x) for x in a.uint8s])
	else: # assume UTF-16
		s = (u''.join([unichr(x) for x in a.uint16s])).encode('utf-8')
	return ('{refs=%d, len=%d, str="%s"}' % (refs, length, s.encode('string_escape')))

def getinfo_for_rtl_String( valobj, dict):
	while valobj.TypeIsPointerType():
		valobj = valobj.Dereference()
	r = valobj.GetChildMemberWithName('refCount').GetValueAsSigned()
	l = valobj.GetChildMemberWithName('length').GetValueAsSigned()
	a = valobj.GetChildMemberWithName('buffer')
	return ret_strinfo(r,l,a)

def getinfo_for_rtl_uString( valobj, dict):
	while valobj.TypeIsPointerType():
		valobj = valobj.Dereference()
	r = valobj.GetChildMemberWithName('refCount').GetValueAsSigned()
	l = valobj.GetChildMemberWithName('length').GetValueAsSigned()
	a = valobj.GetChildMemberWithName('buffer')
	return ret_strinfo(r,l,a)

def getinfo_for__ByteStringData( valobj, dict):
	while valobj.TypeIsPointerType():
		valobj = valobj.Dereference()
	r = valobj.GetChildMemberWithName('mnRefCount').GetValueAsSigned()
	l = valobj.GetChildMemberWithName('mnLen').GetValueAsSigned()
	a = valobj.GetChildMemberWithName('maStr')
	return ret_strinfo(r,l,a)

def getinfo_for__UniStringData( valobj, dict):
	while valobj.TypeIsPointerType():
		valobj = valobj.Dereference()
	r = valobj.GetChildMemberWithName('mnRefCount').GetValueAsSigned()
	l = valobj.GetChildMemberWithName('mnLen').GetValueAsSigned()
	a = valobj.GetChildMemberWithName('maStr')
	return ret_strinfo(r,l,a)


def getinfo_for_rtl_OString( valobj, dict):
	d = valobj.GetChildMemberWithName('pData')
	return d.Dereference()

def getinfo_for_rtl_OUString( valobj, dict):
	d = valobj.GetChildMemberWithName('pData')
	return d.Dereference()

def getinfo_for_ByteString( valobj, dict):
	d = valobj.GetChildMemberWithName('mpData')
	return d.Dereference()

def getinfo_for_UniString( valobj, dict):
	d = valobj.GetChildMemberWithName('mpData')
	return d.Dereference()

