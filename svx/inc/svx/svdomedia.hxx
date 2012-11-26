/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVDOMEDIA_HXX
#define _SVDOMEDIA_HXX

#include <svx/svdorect.hxx>
#include <avmedia/mediaitem.hxx>
#include "svx/svxdllapi.h"

class Graphic;

namespace sdr { namespace contact { class ViewContactOfSdrMediaObj; } }

// ---------------
// - SdrMediaObj -
// ---------------

class SVX_DLLPUBLIC SdrMediaObj : public SdrRectObj
{
private:
    friend class ::sdr::contact::ViewContactOfSdrMediaObj;

protected:
    virtual ~SdrMediaObj();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    SdrMediaObj(SdrModel& rSdrModel, const basegfx::B2DHomMatrix& rTransform = basegfx::B2DHomMatrix());

    virtual bool HasTextEdit() const;
        virtual void                TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
        virtual sal_uInt16              GetObjIdentifier() const;

        virtual void                TakeObjNameSingul(String& rName) const;
        virtual void                TakeObjNamePlural(String& rName) const;

    virtual void AdjustToMaxRange( const basegfx::B2DRange& rMaxRange, bool bShrinkOnly = false );

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
        virtual ::sdr::contact::ViewContact* CreateObjectSpecificViewContact();

private:
        ::avmedia::MediaItem        maMediaProperties;
        ::std::auto_ptr< Graphic >  mapGraphic;
};

#endif //_SVDOMEDIA_HXX
