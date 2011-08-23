/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _UNOEVENT_HXX
#define _UNOEVENT_HXX

#include <bf_svtools/unoevent.hxx>

namespace binfilter {

class SvxMacroItem; 
class SvxMacro; 

class SwXFrame;
class SwXTextFrame;
class SwXTextGraphicObject;
class SwXTextEmbeddedObject;
class SwXFrameStyle;
class SwFmtINetFmt;



class SwHyperlinkEventDescriptor : public SvDetachedEventDescriptor
{
    const ::rtl::OUString sImplName;

    //XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(void) 
        throw( ::com::sun::star::uno::RuntimeException );
protected:
    virtual ~SwHyperlinkEventDescriptor();
public:

     SwHyperlinkEventDescriptor();

    

    void copyMacrosFromINetFmt(const SwFmtINetFmt& aFmt);
    void copyMacrosIntoINetFmt(SwFmtINetFmt& aFmt);

    void copyMacrosFromNameReplace(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XNameReplace> & xReplace);
};



// SwEventDescriptor for 
// 1) SwXTextFrame
// 2) SwXGraphicObject
// 3) SwXEmbeddedObject
// All these objects are an SwXFrame, so they can use a common implementation
class SwFrameEventDescriptor : public SvEventDescriptor
{
    ::rtl::OUString sSwFrameEventDescriptor;

    SwXFrame& rFrame;

public:
    SwFrameEventDescriptor( SwXTextFrame& rFrameRef );
    SwFrameEventDescriptor( SwXTextGraphicObject& rGraphicRef );
    SwFrameEventDescriptor( SwXTextEmbeddedObject& rObjectRef );

    ~SwFrameEventDescriptor();

    virtual ::rtl::OUString SAL_CALL getImplementationName(void) 
        throw( ::com::sun::star::uno::RuntimeException );

protected:
    virtual void setMacroItem(const SvxMacroItem& rItem);
    virtual const SvxMacroItem& getMacroItem();
    virtual sal_uInt16 getMacroItemWhich() const;
};

class SwFrameStyleEventDescriptor : public SvEventDescriptor
{
    ::rtl::OUString sSwFrameStyleEventDescriptor;

    SwXFrameStyle& rStyle;

public:
    SwFrameStyleEventDescriptor( SwXFrameStyle& rStyleRef );

    ~SwFrameStyleEventDescriptor();

    virtual ::rtl::OUString SAL_CALL getImplementationName(void) 
        throw( ::com::sun::star::uno::RuntimeException );

protected:
    virtual void setMacroItem(const SvxMacroItem& rItem);
    virtual const SvxMacroItem& getMacroItem();
    virtual sal_uInt16 getMacroItemWhich() const;
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
