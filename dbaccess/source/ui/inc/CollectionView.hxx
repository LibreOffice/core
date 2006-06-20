/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CollectionView.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:10:44 $
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

#ifndef DBAUI_COLLECTIONVIEW_HXX
#define DBAUI_COLLECTIONVIEW_HXX

#ifndef _SVT_FILEVIEW_HXX
#include <svtools/fileview.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
    /* this class allows to browse through the collection of forms and reports
    */
    class OCollectionView : public ModalDialog
    {
        FixedText       m_aFTCurrentPath;
        ImageButton     m_aNewFolder;
        ImageButton     m_aUp;
        SvtFileView     m_aView;
        FixedText       m_aFTName;
        Edit            m_aName;
        FixedLine       m_aFL;
        PushButton      m_aPB_OK;
        CancelButton    m_aPB_CANCEL;
        HelpButton      m_aPB_HELP;
        String          m_sPath;
        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>                  m_xContent;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;
        Size            m_aDlgSize;
        Size            m_a6Size;
        sal_Int32       m_nFixDeltaHeight;
        sal_Bool        m_bCreateForm;

        DECL_LINK(Up_Click,PushButton*);
        DECL_LINK(NewFolder_Click,PushButton*);
        DECL_LINK(Save_Click,PushButton*);
        DECL_LINK(Dbl_Click_FileView,SvtFileView*);

        /// sets the fixedtext to the right content
        void initCurrentPath();
    public:
        OCollectionView( Window * pParent
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent>& _xContent
                        ,const ::rtl::OUString& _sDefaultName
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB);
        virtual ~OCollectionView();

        ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent> getSelectedFolder() const;
        ::rtl::OUString getName() const;
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................


#endif //DBAUI_COLLECTIONVIEW_HXX


