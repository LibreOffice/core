/*************************************************************************
 *
 *  $RCSfile: CollectionView.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:52:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        String          m_aIniKey;
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


