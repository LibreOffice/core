/*************************************************************************
 *
 *  $RCSfile: tablink.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:50 $
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

#ifndef SC_TABLINK_HXX
#define SC_TABLINK_HXX

#ifndef _LNKBASE_HXX //autogen
#include <so3/lnkbase.hxx>
#endif

#ifndef SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
#define SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
SO2_DECL_REF(SvEmbeddedObject)
#endif

class ScDocShell;
class SfxObjectShell;

class ScTableLink : public SvBaseLink
{
private:
    ScDocShell* pDocShell;      // Container
    String      aFileName;
    String      aFilterName;
    String      aOptions;
    BOOL        bInCreate;
    BOOL        bAddUndo;
    BOOL        bDoPaint;

public:
    TYPEINFO();
    ScTableLink( ScDocShell* pDocSh, const String& rFile,
                    const String& rFilter, const String& rOpt);
    ScTableLink( SfxObjectShell* pShell, const String& rFile,
                    const String& rFilter, const String& rOpt);
    virtual ~ScTableLink();
    virtual void Closed();
    virtual void DataChanged(SvData& rData);

    virtual BOOL Edit(Window* pParent);

    BOOL    Refresh(const String& rNewFile, const String& rNewFilter,
                    const String* pNewOptions = NULL);
    void    SetInCreate(BOOL bSet)      { bInCreate = bSet; }
    void    SetAddUndo(BOOL bSet)       { bAddUndo = bSet; }
    void    SetPaint(BOOL bSet)         { bDoPaint = bSet; }

    const String& GetFileName() const   { return aFileName; }
    const String& GetFilterName() const { return aFilterName; }
    const String& GetOptions() const    { return aOptions; }

    BOOL    IsUsed() const;
};


class ScDocument;
class SfxMedium;

class ScDocumentLoader
{
private:
    ScDocShell*         pDocShell;
    SvEmbeddedObjectRef aRef;
    SfxMedium*          pMedium;

public:
                        ScDocumentLoader( const String& rFileName,
                                            String& rFilterName, String& rOptions,
                                            UINT32 nRekCnt = 0 );
                        ~ScDocumentLoader();
    ScDocument*         GetDocument();
    ScDocShell*         GetDocShell()       { return pDocShell; }
    BOOL                IsError() const;
    String              GetTitle() const;

    static String       GetOptions( SfxMedium& rMedium );
    static void         GetFilterName( const String& rFileName,
                                        String& rFilter, String& rOptions );
};

#endif

