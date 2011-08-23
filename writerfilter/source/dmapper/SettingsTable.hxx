/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SettingsTable.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_SETTINGSTABLE_HXX
#define INCLUDED_SETTINGSTABLE_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <map>

namespace com{ namespace sun{ namespace star{
namespace lang{
class XMultiServiceFactory;
struct Locale;
}
}}}

namespace writerfilter {
namespace dmapper
{
class DomainMapper;
        
using namespace std;
        
struct SettingsTable_Impl;
        
class WRITERFILTER_DLLPRIVATE SettingsTable : public Properties, public Table
{
    SettingsTable_Impl *m_pImpl;
            
 public:
    SettingsTable(  DomainMapper& rDMapper,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xTextFactory
            );
    virtual ~SettingsTable();
            
    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);
            
    // Table
    virtual void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);
            
    //returns default TabStop in 1/100th mm
    int GetDefaultTabStop() const;

    void ApplyProperties( uno::Reference< text::XTextDocument > xDoc );
            
};
typedef boost::shared_ptr< SettingsTable >          SettingsTablePtr;
}}

#endif //
