/*************************************************************************
 *
 *  $RCSfile: digitalsignaturesdialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-13 11:01:59 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _XMLSECURITY_DIGITALSIGNATURESDIALOG_HXX
#define _XMLSECURITY_DIGITALSIGNATURESDIALOG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svx/simptabl.hxx>

#include <xmlsecurity/xmlsignaturehelper.hxx>

#ifndef _STLP_VECTOR
#include <vector>
#endif

namespace com {
namespace sun {
namespace star {
namespace lang {
    class XMultiServiceFactory; }
namespace io {
    class XStream; }
namespace embed {
    class XStorage; }
}}}

namespace css = com::sun::star;
namespace cssu = com::sun::star::uno;

class HeaderBar;

enum DocumentSignatureMode { SignatureModeDocumentContent, SignatureModeMacros, SignatureModePackage };

struct SignatureStreamHelper
{
    cssu::Reference < css::embed::XStorage >    xSignatureStorage;
    cssu::Reference < css::io::XStream >        xSignatureStream;

    void Dispose();
};

class DigitalSignaturesDialog : public ModalDialog
{
private:

    XMLSignatureHelper      maSignatureHelper;

    css::uno::Reference < css::embed::XStorage > mxStore;
    SignatureInformations   aCurrentSignatureInformations;
    bool                    mbVerifySignatures;
    DocumentSignatureMode   meSignatureMode;

    // HACK, until sig in storage works
    rtl::OUString       maSigFileName;

    FixedText           maHintDocFT;
    FixedText           maHintBasicFT;
    FixedText           maHintPackageFT;
    SvxSimpleTable      maSignaturesLB;

    PushButton          maViewBtn;
    PushButton          maAddBtn;
    PushButton          maRemoveBtn;

    FixedLine           maBottomSepFL;
    OKButton            maOKBtn;
    CancelButton        maCancelBtn;
    HelpButton          maHelpBtn;

    DECL_LINK(          ViewButtonHdl, Button* );
    DECL_LINK(          AddButtonHdl, Button* );
    DECL_LINK(          RemoveButtonHdl, Button* );
    DECL_LINK(          SignatureHighlightHdl, void* );
    DECL_LINK(          SignatureSelectHdl, void* );
    DECL_LINK(          StartVerifySignatureHdl, void* );

    void                ImplGetSignatureInformations();
    void                ImplFillSignaturesBox();
    void                ImplShowSignaturesDetails();

public:
    DigitalSignaturesDialog( Window* pParent, cssu::Reference< css::lang::XMultiServiceFactory >& rxMSF, DocumentSignatureMode eMode );
    ~DigitalSignaturesDialog();

            // Initialize the dialog and the security environment, returns TRUE on success
    BOOL    Init( const rtl::OUString& rTokenName );

            // Set the storage which should be signed or verified
    void    SetStorage( const cssu::Reference < css::embed::XStorage >& rxStore );

            // HACK: Set a signature file name, use this until sig in storage works!
    void    SetSignatureFileName( const rtl::OUString& rName ) { maSigFileName = rName; }

            // Execute the dialog...
    short   Execute();

    static SignatureStreamHelper OpenSignatureStream( css::uno::Reference < css::embed::XStorage >& rxStore, sal_Int32 nOpenMode, DocumentSignatureMode eDocSigMode );
    static std::vector< rtl::OUString > CreateElementList( css::uno::Reference < css::embed::XStorage >& rxStore, const ::rtl::OUString rRootStorageName, DocumentSignatureMode eMode );
};

#endif // _XMLSECURITY_DIGITALSIGNATURESDIALOG_HXX

