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


