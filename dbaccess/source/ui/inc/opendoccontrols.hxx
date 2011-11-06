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



#ifndef DBACCESS_SOURCE_UI_INC_OPENDOCCONTROLS_HXX
#define DBACCESS_SOURCE_UI_INC_OPENDOCCONTROLS_HXX

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#include <map>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= OpenDocumentButton
    //====================================================================
    /** a button which can be used to open a document

        The text of the button is the same as for the "Open" command in the application
        UI. Additionally, the icon for this command is also displayed on the button.
    */
    class OpenDocumentButton : public PushButton
    {
    private:
        ::rtl::OUString     m_sModule;

    public:
        OpenDocumentButton( Window* _pParent, const sal_Char* _pAsciiModuleName, const ResId& _rResId );

    protected:
        void    impl_init( const sal_Char* _pAsciiModuleName );
    };

    //====================================================================
    //= OpenDocumentListBox
    //====================================================================
    class OpenDocumentListBox : public ListBox
    {
    private:
        typedef ::std::pair< String, String >       StringPair;
        typedef ::std::map< sal_uInt16, StringPair >    MapIndexToStringPair;

        ::rtl::OUString         m_sModule;
        MapIndexToStringPair    m_aURLs;

    public:
        OpenDocumentListBox( Window* _pParent, const sal_Char* _pAsciiModuleName, const ResId& _rResId );

        String  GetSelectedDocumentURL() const;
        String  GetSelectedDocumentFilter() const;

    protected:
        virtual void        RequestHelp( const HelpEvent& _rHEvt );

        StringPair  impl_getDocumentAtIndex( sal_uInt16 _nListIndex, bool _bSystemNotation = false ) const;

    private:
        void    impl_init( const sal_Char* _pAsciiModuleName );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_SOURCE_UI_INC_OPENDOCCONTROLS_HXX

