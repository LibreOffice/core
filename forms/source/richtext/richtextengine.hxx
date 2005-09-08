/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: richtextengine.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:07:31 $
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
#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTENGINE_HXX
#define FORMS_SOURCE_RICHTEXT_RICHTEXTENGINE_HXX

#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

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

