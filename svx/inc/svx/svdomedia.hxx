/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdomedia.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:24:27 $
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

#ifndef _SVDOMEDIA_HXX
#define _SVDOMEDIA_HXX

#ifndef _SVDORECT_HXX
#include <svx/svdorect.hxx>
#endif
#ifndef _AVMEDIA_MEDIAITEM_HXX
#include <avmedia/mediaitem.hxx>
#endif
#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class Graphic;

namespace sdr { namespace contact { class ViewContactOfSdrMediaObj; } }

// ---------------
// - SdrMediaObj -
// ---------------

class SVX_DLLPUBLIC SdrMediaObj : public SdrRectObj
{
    friend class ::sdr::contact::ViewContactOfSdrMediaObj;

public:

                                    TYPEINFO();

                                    SdrMediaObj();
                                    SdrMediaObj( const Rectangle& rRect );

        virtual                     ~SdrMediaObj();

        virtual FASTBOOL            HasTextEdit() const;

        virtual void                TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
        virtual UINT16              GetObjIdentifier() const;
        virtual sal_Bool            DoPaintObject(XOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;

        virtual void                TakeObjNameSingul(String& rName) const;
        virtual void                TakeObjNamePlural(String& rName) const;

        virtual void                operator=(const SdrObject& rObj);

public:

        void                        setURL( const ::rtl::OUString& rURL );
        const ::rtl::OUString&      getURL() const;

        void                        setMediaProperties( const ::avmedia::MediaItem& rState );
        const ::avmedia::MediaItem& getMediaProperties() const;

        bool                        hasPreferredSize() const;
        Size                        getPreferredSize() const;

        const Graphic&              getGraphic() const;
        void                        setGraphic( const Graphic* pGraphic = NULL );

protected:

        virtual void                mediaPropertiesChanged( const ::avmedia::MediaItem& rNewState );

private:

        virtual ::sdr::contact::ViewContact* CreateObjectSpecificViewContact();

        ::avmedia::MediaItem        maMediaProperties;
        ::std::auto_ptr< Graphic >  mapGraphic;
};

#endif //_SVDOMEDIA_HXX
