# to activate the AOO-LLDB helper script run the line below into LLDB
#	command script import path-to-script/lldb4aoo.py
# or activate it automatically by adding the line to ~/.lldbinit

def __lldb_init_module( dbg, dict):
	# the list of AOO specific types
	aoo_types = ['rtl_String', 'rtl_uString', '_ByteStringData', '_UniStringData']
	pimpl_types = ['rtl::OString', 'rtl::OUString', 'ByteString', 'UniString']
	# register a helper function for each non-trivial type
	for t in aoo_types:
		f = 'getinfo_for_' + t.replace( '::', '_')
		if f in globals():
			dbg.HandleCommand( 'type summary add %s -v -C yes -F %s.%s' % (t,__name__,f))
		else:
			print( 'AOO-LLDB helper function "%s" is not yet defined: '
			    '"%s" types cannot be displayed properly!' % (f,t))
	# register a generic helper function for pimpl types
	dbg.HandleCommand( 'type summary add -F %s.%s -v -C yes -n PIMPL %s' % ( __name__,'get_pimpl_info', ' '.join(pimpl_types)))

# local functions for use by the AOO-type summary providers

def walk_ptrchain( v, info):
	while v.TypeIsPointerType():
		n = v.GetValueAsUnsigned()
		if n == 0:
			info += 'NULL'
			return (None, info)
		else:
			info += '0x%04X-> ' % (n)
			v = v.Dereference()
	return (v, info)

def ret_strdata_info( v, refvar, lenvar, aryvar):
	info = ''
	(v, info) = walk_ptrchain( v, info)
	if not v:
		return info
	r = v.GetChildMemberWithName( refvar).GetValueAsSigned()
	l = v.GetChildMemberWithName( lenvar).GetValueAsSigned()
	c = v.GetChildMemberWithName( aryvar)
	L = min(l,128)
	d = c.AddressOf().GetPointeeData( 0, L)
	if c.GetByteSize() == 1: # assume UTF-8
		s = ''.join([chr(x) for x in d.uint8s])
	else: # assume UTF-16
		s = (u''.join([unichr(x) for x in d.uint16s])).encode('utf-8')
	info += ('{refs=%d, len=%d, str="%s"%s}' % (r, l, s.encode('string_escape'), '...'if(l!=L)else''))
	return info

# definitions for our individual LLDB type summary providers

def get_pimpl_info( valobj, dict):
	v = walk_ptrchain( valobj, '')
	p = v.GetChildAtIndex(0)
	info = v.GetName()
	if v.GetValueAsUnsigned() == 0:
		return '(%s==NULL)' % (info)
	info = '(%s=0x%04X)-> ' % (info,n)
	return info + p.Dereference().GetSummary()


def getinfo_for_rtl_String( valobj, dict):
	return ret_strdata_info( valobj, 'refCount', 'length', 'buffer') 

def getinfo_for_rtl_uString( valobj, dict):
	return ret_strdata_info( valobj, 'refCount', 'length', 'buffer') 

def getinfo_for__ByteStringData( valobj, dict):
	return ret_strdata_info( valobj, 'mnRefCount', 'mnLen', 'maStr') 

def getinfo_for__UniStringData( valobj, dict):
	return ret_strdata_info( valobj, 'mnRefCount', 'mnLen', 'maStr') 


