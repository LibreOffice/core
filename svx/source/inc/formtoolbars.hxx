/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formtoolbars.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:22:34 $
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

#ifndef SVX_SOURCE_INC_FORMTOOLBARS_HXX
#define SVX_SOURCE_INC_FORMTOOLBARS_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef SVX_SOURCE_INC_FMDOCUMENTCLASSIFICATION_HXX
#include "fmdocumentclassification.hxx"
#endif

#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif

#include <tools/solar.h>

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= FormToolboxes
    //====================================================================
    class FormToolboxes
    {
    private:
        DocumentType    m_eDocumentType;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >
                        m_xLayouter;

    public:
        /** constructs an instance
            @param _rxFrame
                the frame to analyze
            @param _eDocType
                the type of the document in <arg>_rxFrame</arg>. May be eUnknownDocumentType, in this
                case, it will be determined automatically.
                If you, for whatever reason, already know the document type, then you can pass it here
                for performance reasons
        */
        FormToolboxes(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame,
            DocumentType _eDocType
        );

    public:
        /** retrieves the URI for the toolbox associated with the given slot, depending
            on the type of our document
        */
        ::rtl::OUString
                getToolboxResourceName( USHORT _nSlotId ) const;

        /** toggles the toolbox associated with the given slot
        */
        void    toggleToolbox( USHORT _nSlotId ) const;

        /** determines whether the toolbox associated with the given slot is currently visible
        */
        bool    isToolboxVisible( USHORT _nSlotId ) const;

        /** ensures that a given toolbox is visible
        */
        inline void showToolbox( USHORT _SlotId ) const
        {
            if ( !isToolboxVisible( _SlotId ) )
                toggleToolbox( _SlotId );
        }
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_SOURCE_INC_FORMTOOLBARS_HXX

