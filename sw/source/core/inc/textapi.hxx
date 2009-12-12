/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textapi.hxx,v $
 * $Revision: 1.4 $
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

#include <svx/unoedsrc.hxx>
#include <svx/unotext.hxx>
#include <svx/eeitem.hxx>
#include <svx/outliner.hxx>
#include <svx/unoipset.hxx>
#include <svx/unoprnms.hxx>
#include <svx/unoforou.hxx>

class SwDoc;

struct SwTextAPIEditSource_Impl;
class SwTextAPIEditSource : public SvxEditSource
{
    SwTextAPIEditSource_Impl* pImpl;

    virtual SvxEditSource*      Clone() const;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();
    explicit            SwTextAPIEditSource( const SwTextAPIEditSource& rSource );

public:
                        SwTextAPIEditSource(SwDoc* pDoc);
    virtual             ~SwTextAPIEditSource();

    void                Dispose();
    void                SetText( OutlinerParaObject& rText );
    void                SetString( const String& rText );
    OutlinerParaObject* CreateText();
    String              GetText();
};

class SwTextAPIObject : public SvxUnoText
{
    SwTextAPIEditSource* pSource;
public:
                        SwTextAPIObject( SwTextAPIEditSource* p);
    virtual             ~SwTextAPIObject() throw();
    void                DisposeEditSource() { pSource->Dispose(); }
    OutlinerParaObject* CreateText() { return pSource->CreateText(); }
    void                SetString( const String& rText ) { pSource->SetString( rText ); }
    void                SetText( OutlinerParaObject& rText ) { pSource->SetText( rText ); }
    String              GetText() { return pSource->GetText(); }
};
