/*************************************************************************
 *
 *  $RCSfile: recent_docs.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:54:10 $
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

#include <cppunit/simpleheader.hxx>

#include "systemshell.hxx"
#include "osl/file.hxx"
#include "osl/security.hxx"
#include "rtl/ustring.hxx"

#include <stdlib.h>
#include <iostream>

//########################################
using namespace osl;
using namespace rtl;

namespace syssh = SystemShell;

//########################################
const OUString SXW_MIME_TYPE = OUString::createFromAscii("application/vnd.sun.xml.writer");
const OUString SXC_MIME_TYPE = OUString::createFromAscii("application/vnd.sun.xml.calc");
const OUString SXI_MIME_TYPE = OUString::createFromAscii("application/vnd.sun.xml.impress");
const OUString SXD_MIME_TYPE = OUString::createFromAscii("application/vnd.sun.xml.draw");
const OUString SXM_MIME_TYPE = OUString::createFromAscii("application/vnd.sun.xml.math");

class Test_AddToRecentDocs : public CppUnit::TestFixture
{
public:

    Test_AddToRecentDocs()
    {
    }

    //------------------------------------------------
    void test_existing_recently_used_file_with_valid_xml()
    {
        system("rm $HOME/.recently-used");
        system("echo '<?xml version=\"1.0\"?>\n<RecentFiles>\n<RecentItem>\n<URI>file:///home/federico/gedit.txt</URI>\n<Mime-Type>text/plain</Mime-Type>\n<Timestamp>1046485966</Timestamp>\n<Groups>\n<Group>gedit</Group>\n</Groups>\n</RecentItem>\n</RecentFiles>' > $HOME/.recently-used");

        rtl::OUString url = rtl::OUString::createFromAscii("file:///home_athene/test.sxw");
        syssh::AddToRecentDocumentList(url, SXW_MIME_TYPE);

        url = rtl::OUString::createFromAscii("file:///home_athene/test.sxc");
        syssh::AddToRecentDocumentList(url, SXC_MIME_TYPE);

        url = rtl::OUString::createFromAscii("file:///home_athene/test.sxi");
        syssh::AddToRecentDocumentList(url, SXI_MIME_TYPE);

        url = rtl::OUString::createFromAscii("file:///home_athene/test.sxd");
        syssh::AddToRecentDocumentList(url, SXD_MIME_TYPE);

        url = rtl::OUString::createFromAscii("file:///home_athene/test.sxm");
        syssh::AddToRecentDocumentList(url, SXM_MIME_TYPE);
    }

    //------------------------------------------------
    void test_existing_recently_used_file_with_invalid_xml()
    {
        system("rm $HOME/.recently-used");
        system("echo '<?xml version=\"1.0\"?>\n<RecentFiles>\n<RecentItem>\n<URI>file:///home/federico/gedit.txt</URI>\n<Mime-Type>text/plain</Mime-Type>\n<Timestamp>1046485966</Timestamp>\n<Groups>\n<Group>gedit</Group>\n</Groups>\n<RecentItem>\n<URI>file:///home/federico/gedit2.txt</URI>\n<Mime-Type>text/plain</Mime-Type>\n<Timestamp>1046485966</Timestamp>\n<Groups>\n<Group>gedit</Group>\n</Groups>\n</RecentItem>\n</RecentFiles>' > $HOME/.recently-used");

        rtl::OUString url = rtl::OUString::createFromAscii("file:///home_athene/test.sxw");
        syssh::AddToRecentDocumentList(url, SXW_MIME_TYPE);
    }

    //------------------------------------------------
    void test_non_existing_recently_used_file()
    {
        int ret = system("rm $HOME/.recently-used");

        rtl::OUString url = rtl::OUString::createFromAscii("file:///home_athene/test.sxw");
        syssh::AddToRecentDocumentList(url, SXW_MIME_TYPE);

        url = rtl::OUString::createFromAscii("file:///home_athene/test.sxc");
        syssh::AddToRecentDocumentList(url, SXC_MIME_TYPE);

        url = rtl::OUString::createFromAscii("file:///home_athene/test.sxi");
        syssh::AddToRecentDocumentList(url, SXI_MIME_TYPE);

        url = rtl::OUString::createFromAscii("file:///home_athene/test.sxd");
        syssh::AddToRecentDocumentList(url, SXD_MIME_TYPE);

        url = rtl::OUString::createFromAscii("file:///home_athene/test.sxm");
        syssh::AddToRecentDocumentList(url, SXM_MIME_TYPE);

        ret = system("ls $HOME/.recently-used");

        CPPUNIT_ASSERT_MESSAGE(".recently-used doesn't exist", 0 == ret);
    }

    //------------------------------------------------
    OUString get_recently_used_url()
    {
        Security sec;
        OUString recently_used_url;

        CPPUNIT_ASSERT_MESSAGE("cannot get user home directory", sec.getHomeDir(recently_used_url));

        if ((recently_used_url.getLength() > 0) &&
            ('/' != recently_used_url.pData->buffer[recently_used_url.getLength() - 1]))
            recently_used_url += rtl::OUString::createFromAscii("/");

        recently_used_url += rtl::OUString::createFromAscii(".recently-used");
        return recently_used_url;
    }

    //------------------------------------------------
    void read_recently_used(void* buffer, size_t size)
    {
        File ruf(get_recently_used_url());
        FileBase::RC rc = ruf.open(OpenFlag_Read);

        CPPUNIT_ASSERT_MESSAGE("Cannot open ~/.recently-used", FileBase::E_None == rc);

        sal_uInt64 read;
        ruf.read(buffer, size, read);
        ruf.close();
        CPPUNIT_ASSERT_MESSAGE("Cannot read .recently-used", (read > 0));
    }

    //------------------------------------------------
    void test_existing_recently_used_file_without_xml()
    {
        // remove an existing .recently-used file
        // create a new one with non xml data
        system("rm $HOME/.recently-used");
        system("date > $HOME/.recently-used");

        char cnt_before[128];
        memset(cnt_before, 0, sizeof(cnt_before));
        read_recently_used(cnt_before, sizeof(cnt_before));

        OUString url = OUString::createFromAscii("file:///home_athene/test.sxw");
        syssh::AddToRecentDocumentList(url, SXW_MIME_TYPE);

        char cnt_after[128];
        memset(cnt_after, 0, sizeof(cnt_after));
        read_recently_used(cnt_after, sizeof(cnt_after));
        CPPUNIT_ASSERT_MESSAGE(".recently-used unexpectly touched", (0 == strcmp(cnt_before, cnt_after)));
    }

    //------------------------------------------------
    void test_existing_recently_used_with_unknown_xml()
    {
        system("rm $HOME/.recently-used");

        char* xml_unknown = "<?xml version=\"1.0\"?>\n<test>test</test>";
        char cmd[256];
        sprintf(cmd, "echo '%s' > $HOME/.recently-used", xml_unknown);
        system(cmd);

        OUString url = OUString::createFromAscii("file:///home_athene/test.sxw");
        syssh::AddToRecentDocumentList(url, SXW_MIME_TYPE);

        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        read_recently_used(buffer, sizeof(buffer));

        CPPUNIT_ASSERT_MESSAGE("Unexpected recently-used content", (0 == strncmp(xml_unknown, buffer, strlen(xml_unknown))));
    }

    //------------------------------------------------
    void test_existing_empty_recently_used_file()
    {
        system("rm $HOME/.recently-used");
        system("touch $HOME/.recently-used");

        OUString url = OUString::createFromAscii("file:///home_athene/test.sxw");
        syssh::AddToRecentDocumentList(url, SXW_MIME_TYPE);

        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        read_recently_used(buffer, sizeof(buffer));

        char* expected = "<?xml version=\"1.0\"?>\n<RecentFiles>";
        CPPUNIT_ASSERT_MESSAGE("Unexpected recently-used content", (0 == strncmp(buffer, expected, strlen(expected))));
    }

    //------------------------------------------------
    CPPUNIT_TEST_SUITE(Test_AddToRecentDocs);
    CPPUNIT_TEST(test_existing_recently_used_file_with_valid_xml);
    CPPUNIT_TEST(test_non_existing_recently_used_file);
    CPPUNIT_TEST(test_existing_recently_used_file_without_xml);
    CPPUNIT_TEST(test_existing_recently_used_with_unknown_xml);
    CPPUNIT_TEST(test_existing_empty_recently_used_file);
    CPPUNIT_TEST(test_existing_recently_used_file_with_invalid_xml);
    CPPUNIT_TEST_SUITE_END();
};


//#####################################
// register test suites
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test_AddToRecentDocs, "Test_AddToRecentDocs");

NOADDITIONAL;

