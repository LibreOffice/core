/*************************************************************************
 *
 *  $RCSfile: TemplateScanner.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 10:57:38 $
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

#include "TemplateScanner.hxx"

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#ifndef _VOS_MUTEX_HXX
#include <vos/mutex.hxx>
#endif

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

using namespace ::rtl;
using namespace ::ucb;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::ucb;

namespace {
const OUString TITLE            = OUString::createFromAscii ("Title");
const OUString TARGET_DIR_URL   = OUString::createFromAscii ("TargetDirURL");
const OUString DESCRIPTION      = OUString::createFromAscii ("TypeDescription");
const OUString TARGET_URL       = OUString::createFromAscii ("TargetURL");

const OUString DOCTEMPLATES     = OUString::createFromAscii ("com.sun.star.frame.DocumentTemplates");

//  These strings are used to find impress templates in the tree of
//  template files.  Should probably be determined dynamically.
const OUString IMPRESS_BIN_TEMPLATE = OUString::createFromAscii ("application/vnd.stardivision.impress");
const OUString IMPRESS_XML_TEMPLATE = OUString::createFromAscii ("application/vnd.sun.xml.impress");
// The following id comes from the bugdoc in #i2764#.
const OUString IMPRESS_XML_TEMPLATE_B = OUString::createFromAscii ("Impress 2.0");
}

namespace sd
{

TemplateScanner::TemplateScanner (void)
{
    //  empty;
}




TemplateScanner::~TemplateScanner (void)
{
    // Delete all entries of the template list that have not been
    // transferred to another object.
    std::vector<TemplateDir*>::iterator I;
    for (I=maFolderList.begin(); I!=maFolderList.end(); I++)
        if (*I != NULL)
            delete *I;
}




void TemplateScanner::GetTemplateRoot (void)
{
    Reference<lang::XMultiServiceFactory> xFactory =
        ::comphelper::getProcessServiceFactory ();

    Reference<frame::XDocumentTemplates> xTemplates (
        xFactory->createInstance (DOCTEMPLATES), UNO_QUERY);

    if (xTemplates.is())
        mxTemplateRoot = xTemplates->getContent();
}




void TemplateScanner::ScanEntries (Content& rRoot, TemplateDir* pDir)
{
    Reference<XCommandEnvironment> aCmdEnv;

    //  We are interested only in three properties: the entry's name,
    //  its URL, and its content type.
    Sequence<OUString> aProps (3);
    aProps[0] = TITLE;
    aProps[1] = TARGET_URL;
    aProps[2] = DESCRIPTION;

    //  Create a cursor to iterate over the templates in this folders.
    ResultSetInclude eInclude = INCLUDE_DOCUMENTS_ONLY;
    Reference<XResultSet> xResultSet (rRoot.createCursor (aProps, eInclude));

    if (xResultSet.is())
    {
        Reference<XContentAccess> xContentAccess (xResultSet, UNO_QUERY);
        Reference<XRow> xRow (xResultSet, UNO_QUERY);

        //  Iterate over the template folder.
        while (xResultSet->next())
        {
            OUString aTitle (xRow->getString (1));
            OUString aTargetURL (xRow->getString (2));
            OUString aContentType (xRow->getString (3));

            OUString aId = xContentAccess->queryContentIdentifierString();
            Content  aContent = Content (aId, aCmdEnv);
            if ( ! aContent.isDocument ())
                continue;

            //  Check wether the entry is an impress template.  If so add a
            //  new entry to the resulting list (which is created first if
            //  necessary).
            if (    (aContentType == IMPRESS_BIN_TEMPLATE)
                ||  (aContentType == IMPRESS_XML_TEMPLATE)
                ||  (aContentType == IMPRESS_XML_TEMPLATE_B))
                pDir->m_aEntries.push_back (
                    new TemplateEntry (aTitle, aTargetURL));
        }
    }
}




void TemplateScanner::ScanFolders (void)
{
    try
    {
        //  Create content for template folders.
        Reference<XCommandEnvironment> aCmdEnv;
        Content aTemplateDir (mxTemplateRoot, aCmdEnv);

        //  Define the list of properties we are interested in.
        Sequence<OUString> aProps (2);
        aProps[0] = TITLE;
        aProps[1] = TARGET_DIR_URL;

        //  Create an cursor to iterate over the template folders.
        ResultSetInclude eInclude = INCLUDE_FOLDERS_ONLY;
        Reference<XResultSet> xResultSet (aTemplateDir.createCursor (
            aProps, eInclude));

        if (xResultSet.is())
        {
            Reference<XContentAccess> xContentAccess (xResultSet, UNO_QUERY);
            Reference<XRow> xRow (xResultSet, UNO_QUERY);

            while (xResultSet->next())
            {
                OUString aTitle (xRow->getString (1));
                OUString aTargetDir (xRow->getString (2));

                OUString aId = xContentAccess->queryContentIdentifierString();
                Content  aContent = Content (aId, aCmdEnv);
                if (aContent.isFolder())
                {
                    //  Scan the folder and insert it into the list of
                    //  template folders.
                    TemplateDir* pDir = new TemplateDir (aTitle, aTargetDir);
                    if (pDir != NULL)
                    {
                        ScanEntries (aContent, pDir);
                        if (pDir->m_aEntries.empty())
                            delete pDir;
                        else
                        {
                            ::vos::OGuard aGuard(Application::GetSolarMutex());
                            maFolderList.push_back(pDir);
                        }
                    }
                }
            }
        }
    }
    catch(::com::sun::star::uno::Exception&)
    {
        //  Ignore all exceptions.
    }
}




void TemplateScanner::Scan (void)
{
    GetTemplateRoot ();
    ScanFolders ();
}




std::vector<TemplateDir*>& TemplateScanner::GetFolderList (void)
{
    return maFolderList;
}

}
