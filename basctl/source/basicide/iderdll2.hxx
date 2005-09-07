/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: iderdll2.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:04:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _IDERDLL2_HXX
#define _IDERDLL2_HXX

class StarBASIC;
class SvxSearchItem;
class Accelerator;

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif

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

    USHORT          nBasicDialogCount;

    BOOL            OLD_bRelMacroRecording;
    BOOL            bChoosingMacro;
    BOOL            bShellInCriticalSection;

    void            InitAccelerator();

protected:
    DECL_LINK( GlobalBasicBreakHdl, StarBASIC * );

public:
                            BasicIDEData();
                            ~BasicIDEData();

    LibInfos&               GetLibInfos() { return aLibInfos; }

    BasicEntryDescriptor&   GetLastEntryDescriptor()    { return m_aLastEntryDesc; }
    void                    SetLastEntryDescriptor( BasicEntryDescriptor& rDesc ) { m_aLastEntryDesc = rDesc; }

    BOOL&                   ChoosingMacro()             { return bChoosingMacro; }
    BOOL&                   ShellInCriticalSection()    { return bShellInCriticalSection; }

    USHORT                  GetBasicDialogCount() const { return nBasicDialogCount; }
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

    Accelerator*            GetAccelerator()
                            {
                                if ( !pAccelerator )
                                    InitAccelerator();
                                return pAccelerator;
                            }

    DECL_LINK( ExecuteMacroEvent, void * );
};


#endif //_IDERDLL2_HXX
