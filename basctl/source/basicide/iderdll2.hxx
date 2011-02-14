/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _IDERDLL2_HXX
#define _IDERDLL2_HXX

class StarBASIC;
class SvxSearchItem;
class Accelerator;

#include <tools/string.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>

#include <bastypes.hxx>
#include <bastype2.hxx>

#define INVPOSITION 0x7fff

class BasicIDEData
{
private:
    Accelerator*    pAccelerator;
    SvxSearchItem*  pSearchItem;

    LibInfos        aLibInfos;

    BasicEntryDescriptor    m_aLastEntryDesc;

    Point           aObjCatPos;
    Size            aObjCatSize;

    String          aAddLibPath;
    String          aAddLibFilter;

    sal_uInt16          nBasicDialogCount;

    sal_Bool            OLD_bRelMacroRecording;
    sal_Bool            bChoosingMacro;
    sal_Bool            bShellInCriticalSection;

protected:
    DECL_LINK( GlobalBasicBreakHdl, StarBASIC * );

public:
                            BasicIDEData();
                            ~BasicIDEData();

    LibInfos&               GetLibInfos() { return aLibInfos; }

    BasicEntryDescriptor&   GetLastEntryDescriptor()    { return m_aLastEntryDesc; }
    void                    SetLastEntryDescriptor( BasicEntryDescriptor& rDesc ) { m_aLastEntryDesc = rDesc; }

    sal_Bool&                   ChoosingMacro()             { return bChoosingMacro; }
    sal_Bool&                   ShellInCriticalSection()    { return bShellInCriticalSection; }

    sal_uInt16                  GetBasicDialogCount() const { return nBasicDialogCount; }
    void                    IncBasicDialogCount()       { nBasicDialogCount++; }
    void                    DecBasicDialogCount()       { nBasicDialogCount--; }

    SvxSearchItem&          GetSearchItem() const;
    void                    SetSearchItem( const SvxSearchItem& rItem );

    void                    SetObjectCatalogPos( const Point& rPnt )
                                { aObjCatPos = rPnt; }
    const Point&            GetObjectCatalogPos() const
                                { return aObjCatPos; }

    void                    SetObjectCatalogSize( const Size& rSize )
                                { aObjCatSize = rSize; }
    const Size&             GetObjectCatalogSize() const
                                { return aObjCatSize; }

    const String&           GetAddLibPath() const   { return aAddLibPath; }
    void                    SetAddLibPath( const String& rPath ) { aAddLibPath = rPath; }

    const String&           GetAddLibFilter() const { return aAddLibFilter; }
    void                    SetAddLibFilter( const String& rFilter )  { aAddLibFilter = rFilter; }
};


#endif //_IDERDLL2_HXX
