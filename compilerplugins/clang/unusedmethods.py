#!/usr/bin/python

import sys

definitionSet = set()
callSet = set()
# things we need to exclude for reasons like :
# - it's a weird template thingy that confuses the plugin
exclusionSet = set([
    "double basegfx::DoubleTraits::maxVal()",
    "double basegfx::DoubleTraits::minVal()",
    "double basegfx::DoubleTraits::neutral()",
    "int basegfx::Int32Traits::maxVal()",
    "int basegfx::Int32Traits::minVal()",
    "int basegfx::Int32Traits::neutral()",
    "unsigned long UniqueIndexImpl::Insert(void *)",
    # used from a yacc lexer
    "class rtl::OUString unoidl::detail::SourceProviderType::getName() const",
    "_Bool unoidl::detail::SourceProviderType::equals(const struct unoidl::detail::SourceProviderType &) const",
    "_Bool unoidl::detail::SourceProviderEntityPad::isPublished() const",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::checkMemberClashes(int,void *,struct unoidl::detail::SourceProviderScannerData *,const class rtl::OUString &,const class rtl::OUString &,_Bool) const",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::checkBaseClashes(int,void *,struct unoidl::detail::SourceProviderScannerData *,const class rtl::OUString &,const class rtl::Reference<class unoidl::InterfaceTypeEntity> &,_Bool,_Bool,_Bool,class std::__debug::set<class rtl::OUString, struct std::less<class rtl::OUString>, class std::allocator<class rtl::OUString> > *) const",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::addDirectBase(int,void *,struct unoidl::detail::SourceProviderScannerData *,const struct unoidl::detail::SourceProviderInterfaceTypeEntityPad::DirectBase &,_Bool)",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::addBase(int,void *,struct unoidl::detail::SourceProviderScannerData *,const class rtl::OUString &,const class rtl::OUString &,const class rtl::Reference<class unoidl::InterfaceTypeEntity> &,_Bool,_Bool)",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::addDirectMember(int,void *,struct unoidl::detail::SourceProviderScannerData *,const class rtl::OUString &)",
    "_Bool unoidl::detail::SourceProviderInterfaceTypeEntityPad::addOptionalBaseMembers(int,void *,struct unoidl::detail::SourceProviderScannerData *,const class rtl::OUString &,const class rtl::Reference<class unoidl::InterfaceTypeEntity> &)",
    "void unoidl::detail::SourceProviderScannerData::setSource(const void *,unsigned long)",
    ])


with open(sys.argv[1]) as txt:
    for line in txt:
    
        if line.startswith("definition:\t"):
            idx1 = line.find("\t")
            clazzName = line[idx1+1 : len(line)-1]
            definitionSet.add(clazzName)
            
        elif line.startswith("call:\t"):
            idx1 = line.find("\t")
            clazzName = line[idx1+1 : len(line)-1]
            callSet.add(clazzName)
            
for clazz in sorted(definitionSet - callSet - exclusionSet):
    # ignore operators, they are normally called from inside STL code
    if ((clazz.find("::operator") == -1)
        # ignore the custom RTTI stuff
        and (clazz.find("::CreateType()") == -1)
        and (clazz.find("::IsA(") == -1)
        and(clazz.find("::Type()") == -1)) :
        print clazz

# add an empty line at the end to make it easier for the unusedmethodsremove plugin to mmap() the output file 
print
        

