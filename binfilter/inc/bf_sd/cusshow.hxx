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

#ifndef _SD_CUSSHOW_HXX
#define _SD_CUSSHOW_HXX


#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif


#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
namespace binfilter {

class SdDrawDocument;

/*************************************************************************
|*
|* CustomShow
|*
\************************************************************************/
class SdCustomShow : public List
{
private:
    String          aName;
    SdDrawDocument* pDoc;

    // this is a weak reference to a possible living api wrapper for this custom show
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > mxUnoCustomShow;

public:
    SdCustomShow(SdDrawDocument* pDrawDoc);
    SdCustomShow(SdDrawDocument* pDrawDoc, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xShow );

    SdCustomShow( const SdCustomShow& rShow );
    virtual ~SdCustomShow();

    void   SetName(const String& rName) { aName = rName; }
    String GetName() const              { return aName; }

    SdDrawDocument* GetDoc() const { return pDoc; }

    friend SvStream& operator << (SvStream& rOut, const SdCustomShow& rCustomShow);
    friend SvStream& operator >> (SvStream& rIn, SdCustomShow& rCustomShow);

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoCustomShow();
};

} //namespace binfilter
#endif      // _SD_CUSSHOW_HXX

