# to activate LLDB helper script run the command below when inside LLDB
#	command script import /tools/lldb4aoo.py
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

def ret_strdata_info( v, refvar, lenvar, aryvar):
	while v.TypeIsPointerType():
		if v.GetValueAsUnsigned() == 0:
			return 'NULL-Pointer!'
		v = v.Dereference()
	r = v.GetChildMemberWithName( refvar).GetValueAsSigned()
	l = v.GetChildMemberWithName( lenvar).GetValueAsSigned()
	c = v.GetChildMemberWithName( aryvar)
	d = c.AddressOf().GetPointeeData( 0, l)
	if c.GetByteSize() == 1: # assume UTF-8
		s = ''.join([chr(x) for x in d.uint8s])
	else: # assume UTF-16
		s = (u''.join([unichr(x) for x in d.uint16s])).encode('utf-8')
	info = ('{refs=%d, len=%d, str="%s"}' % (r, l, s.encode('string_escape')))
	return info

def ret_strobject_info( v, ptrvar):
	while v.TypeIsPointerType():
		if v.GetValueAsUnsigned() == 0:
			return 'NULL-Pointer!'
		v = v.Dereference()
	p = v.GetChildMemberWithName( ptrvar)
	return p.Dereference()


def getinfo_for_rtl_String( valobj, dict):
	return ret_strdata_info( valobj, 'refCount', 'length', 'buffer') 

def getinfo_for_rtl_uString( valobj, dict):
	return ret_strdata_info( valobj, 'refCount', 'length', 'buffer') 

def getinfo_for__ByteStringData( valobj, dict):
	return ret_strdata_info( valobj, 'mnRefCount', 'mnLen', 'maStr') 

def getinfo_for__UniStringData( valobj, dict):
	return ret_strdata_info( valobj, 'mnRefCount', 'mnLen', 'maStr') 


def getinfo_for_rtl_OString( valobj, dict):
	return ret_strobject_info( valobj, 'pData')

def getinfo_for_rtl_OUString( valobj, dict):
	return ret_strobject_jinfo( valobj, 'pData')

def getinfo_for_ByteString( valobj, dict):
	return ret_strobject_jinfo( valobj, 'mpData')

def getinfo_for_UniString( valobj, dict):
	return ret_strobject_info( valobj, 'mpData')

