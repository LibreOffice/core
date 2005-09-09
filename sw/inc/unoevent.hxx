/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unoevent.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:25:23 $
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
#ifndef _UNOEVENT_HXX
#define _UNOEVENT_HXX

#ifndef _SVTOOLS_UNOEVENT_HXX_
#include <svtools/unoevent.hxx>
#endif

#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif


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
    virtual rtl::OUString SAL_CALL getImplementationName(void)
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

    virtual rtl::OUString SAL_CALL getImplementationName(void)
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

    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );

protected:
    virtual void setMacroItem(const SvxMacroItem& rItem);
    virtual const SvxMacroItem& getMacroItem();
    virtual sal_uInt16 getMacroItemWhich() const;
};


#endif
