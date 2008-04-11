/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: richtextengine.hxx,v $
 * $Revision: 1.5 $
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
#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTENGINE_HXX
#define FORMS_SOURCE_RICHTEXT_RICHTEXTENGINE_HXX

#include <svx/editeng.hxx>
#include <tools/link.hxx>

#include <vector>

class SfxItemPool;
class EditStatus;
//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= IEngineStatusListener
    //====================================================================
    class IEngineStatusListener
    {
    public:
        virtual void EditEngineStatusChanged( const EditStatus& _rStatus ) = 0;
    };

    //====================================================================
    //= RichTextEngine
    //====================================================================
    class RichTextEngine : public EditEngine
    {
    private:
        SfxItemPool*                            m_pEnginePool;
        ::std::vector< IEngineStatusListener* > m_aStatusListeners;

    public:
        static  RichTextEngine* Create();
                RichTextEngine* Clone();

                ~RichTextEngine( );

        // for multiplexing the StatusChanged events of the edit engine
        void registerEngineStatusListener( IEngineStatusListener* _pListener );
        void revokeEngineStatusListener( IEngineStatusListener* _pListener );

        inline SfxItemPool* getPool() { return m_pEnginePool; }

    protected:
        /** constructs a new RichTextEngine. The instances takes the ownership of the given SfxItemPool
        */
        RichTextEngine( SfxItemPool* _pPool );

    private:
        RichTextEngine( );                                  // never implemented
        RichTextEngine( const RichTextEngine& );            // never implemented
        RichTextEngine& operator=( const RichTextEngine& ); // never implemented

    private:
        DECL_LINK( EditEngineStatusChanged, EditStatus* );
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_SOURCE_RICHTEXT_RICHTEXTENGINE_HXX

