/*************************************************************************
 *
 *  $RCSfile: dpsdbtab.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2000-10-19 15:23:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_DPSDBTAB_HXX
#define SC_DPSDBTAB_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}

#ifndef SC_DPTABDAT_HXX
#include "dptabdat.hxx"
#endif

// --------------------------------------------------------------------
//
//  implementation of ScDPTableData with database data
//

struct ScImportSourceDesc
{
    String  aDBName;
    String  aObject;
    USHORT  nType;          // enum DataImportMode
    BOOL    bNative;

    BOOL operator== ( const ScImportSourceDesc& rOther ) const
        { return aDBName == rOther.aDBName &&
                 aObject == rOther.aObject &&
                 nType   == rOther.nType &&
                 bNative == rOther.bNative; }
};

class ScDatabaseDPData_Impl;

class ScDatabaseDPData : public ScDPTableData
{
private:
    ScDatabaseDPData_Impl* pImpl;

    BOOL            OpenDatabase();
    void            InitAllColumnEntries();

public:
                    ScDatabaseDPData(
                        ::com::sun::star::uno::Reference<
                            ::com::sun::star::lang::XMultiServiceFactory > xSMgr,
                        const ScImportSourceDesc& rImport );
    virtual         ~ScDatabaseDPData();

    virtual long                    GetColumnCount();
    virtual const TypedStrCollection&   GetColumnEntries(long nColumn);
    virtual String                  getDimensionName(long nColumn);
    virtual BOOL                    getIsDataLayoutDimension(long nColumn);
    virtual BOOL                    IsDateDimension(long nDim);
    virtual void                    DisposeData();
    virtual void                    SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty );

    virtual void                    ResetIterator();
    virtual BOOL                    GetNextRow( const ScDPTableIteratorParam& rParam );
};



#endif

