/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textapi.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:43:50 $
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

#include <svx/unoedsrc.hxx>
#include <svx/unotext.hxx>
#include <svx/eeitem.hxx>

struct SwTextAPIEditSource_Impl;
class SwTextAPIEditSource : public SvxEditSource
{
    SwTextAPIEditSource_Impl* pImpl;

    virtual SvxEditSource*      Clone() const;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();
    explicit            SwTextAPIEditSource( const SwTextAPIEditSource& rSource );

public:
                        SwTextAPIEditSource(SfxItemPool* pPool);
    virtual             ~SwTextAPIEditSource();

    void                Dispose();
    void                SetText( OutlinerParaObject& rText );
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
    void                SetText( OutlinerParaObject& rText ) { pSource->SetText( rText ); }
    String              GetText() { return pSource->GetText(); }
};
