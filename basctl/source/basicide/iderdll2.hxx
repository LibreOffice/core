/*************************************************************************
 *
 *  $RCSfile: iderdll2.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:37 $
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

#define INVPOSITION 0x7fff

class BasicIDEData
{
private:
    Accelerator*    pAccelerator;
    SvxSearchItem*  pSearchItem;

    LibInfos        aLibInfos;

    String          TheLastMacroInMacroDlg;

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
    DECL_LINK( GlobalBasicErrorHdl, StarBASIC * );
    DECL_LINK( GlobalBasicBreakHdl, StarBASIC * );

public:
                            BasicIDEData();
                            ~BasicIDEData();

    LibInfos&               GetLibInfos() { return aLibInfos; }

    String&                 GetLastMacro()              { return TheLastMacroInMacroDlg; }
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
