/*************************************************************************
 *
 *  $RCSfile: arealink.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dr $ $Date: 2001-04-05 10:42:48 $
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

#ifndef SC_AREALINK_HXX
#define SC_AREALINK_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef _LNKBASE_HXX //autogen
#include <so3/lnkbase.hxx>
#endif

class ScDocShell;
class SfxObjectShell;

class ScAreaLink : public ::so3::SvBaseLink
{
private:
    ScDocShell* pDocShell;      // Container
    String      aFileName;
    String      aFilterName;
    String      aOptions;
    String      aSourceArea;
    ScRange     aDestArea;
    ULONG       nRefreshDelay;  // refresh delay in seconds; 0 = off
    BOOL        bAddUndo;
    BOOL        bInCreate;
    BOOL        bDoInsert;      // wird fuer das erste Update auf FALSE gesetzt

public:
    TYPEINFO();
    ScAreaLink( SfxObjectShell* pShell, const String& rFile,
                    const String& rFilter, const String& rOpt,
                    const String& rArea, const ScRange& rDest, ULONG nRefresh );
    virtual ~ScAreaLink();

    virtual void Closed();
    virtual void DataChanged( const String& rMimeType,
                              const ::com::sun::star::uno::Any & rValue );

    virtual BOOL Edit(Window* pParent);

    BOOL    Refresh( const String& rNewFile, const String& rNewFilter,
                    const String& rNewArea, ULONG nNewRefresh );

    void    SetInCreate(BOOL bSet)                  { bInCreate = bSet; }
    void    SetDoInsert(BOOL bSet)                  { bDoInsert = bSet; }
    void    SetDestArea(const ScRange& rNew);
    void    SetRefreshDelay(ULONG nRefresh)         { nRefreshDelay = nRefresh; }
    void    SetSource(const String& rDoc, const String& rFlt, const String& rOpt,
                        const String& rArea);

    BOOL    IsEqual( const String& rFile, const String& rFilter, const String& rOpt,
                        const String& rSource, const ScRange& rDest ) const;

    const String&   GetFile() const         { return aFileName;     }
    const String&   GetFilter() const       { return aFilterName;   }
    const String&   GetOptions() const      { return aOptions;      }
    const String&   GetSource() const       { return aSourceArea;   }
    const ScRange&  GetDestArea() const     { return aDestArea;     }
    ULONG           GetRefreshDelay() const { return nRefreshDelay; }
};


#endif
