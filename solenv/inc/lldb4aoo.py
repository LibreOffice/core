# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

# to activate the AOO-LLDB helper script type the line below into LLDB
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

	# add info about specific helper methods
	# assume functions with docstrings are available for general consumption
	helper_funcs = [v for (k,v) in globals().iteritems() if( not k.startswith('_') and callable(v) and v.__doc__)]
	if helper_funcs:
		print( 'Available AOO-specific helper functions:')
		for hfunc in helper_funcs:
			shortdesc = hfunc.__doc__.splitlines()[0]
			print( '\t%s\t# "%s"' %(hfunc.__name__, shortdesc))
		print( 'Run them with:')
		for hfunc in helper_funcs[:4]:
			print( '\tscript %s.%s()' %(__name__, hfunc.__name__))

# some helpers for use from interactive LLDB sessions

import lldb

def add_breakpoints():
	'Setup breakpoints useful for AOO debugging'
	dbg = lldb.debugger
	if dbg.GetNumTargets() == 0:
		return
	# the list of interesting function breakpoints
	aoo_breakfn = ['main', '__cxa_call_unexpected', 'objc_exception_throw']
	aoo_breakfn += ['__cxa_throw']
	# register breakpoints for function basenames
	for b in aoo_breakfn:
		dbg.HandleCommand( 'breakpoint set -b ' + b)


# local functions for use by the AOO-type summary providers

def walk_ptrchain( v):
	info = ''
	while v.TypeIsPointerType():
		n = v.GetValueAsUnsigned()
		if n == 0:
			info += 'NULL'
			return (None, info)
		info += '0x%04X-> ' % (n)
		v = v.Dereference()
	return (v, info)

def ret_strdata_info( v, refvar, lenvar, aryvar):
	(v, info) = walk_ptrchain( v)
	if not v:
		return info
	r = v.GetChildMemberWithName( refvar).GetValueAsSigned()
	l = v.GetChildMemberWithName( lenvar).GetValueAsSigned()
	c = v.GetChildMemberWithName( aryvar)
	if (r < 0) or (l < 0):
		info += 'CORRUPT_STR={refs=%d, len=%d}' % (r,l)
		return info
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
	(v, info) = walk_ptrchain( valobj)
	p = v.GetChildAtIndex(0)
	pname = p.GetName()
	n = p.GetValueAsUnsigned()
	if n == 0:
		return '%s(%s==NULL)' % (info, pname)
	info = '%s(%s=0x%04X)-> ' % (info, pname, n)
	return info + p.Dereference().GetSummary()


def getinfo_for_rtl_String( valobj, dict):
	return ret_strdata_info( valobj, 'refCount', 'length', 'buffer') 

def getinfo_for_rtl_uString( valobj, dict):
	return ret_strdata_info( valobj, 'refCount', 'length', 'buffer') 

def getinfo_for__ByteStringData( valobj, dict):
	return ret_strdata_info( valobj, 'mnRefCount', 'mnLen', 'maStr') 

def getinfo_for__UniStringData( valobj, dict):
	return ret_strdata_info( valobj, 'mnRefCount', 'mnLen', 'maStr') 

