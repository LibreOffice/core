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



#include "gtest/gtest.h"
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Duration.hpp>

#include <sfx2/Metadatable.hxx>
#include <sfx2/XmlIdRegistry.hxx>


using namespace ::com::sun::star;


namespace {

class MetadatableTest
    : public ::testing::Test
{
public:
    virtual void SetUp();
    virtual void TearDown();
};

void MetadatableTest::SetUp()
{
}

void MetadatableTest::TearDown()
{
}


class MockMetadatable
    : public ::sfx2::Metadatable
{
private:
    ::sfx2::IXmlIdRegistry & m_rRegistry;

public:
    MockMetadatable(::sfx2::IXmlIdRegistry & i_rReg,
            bool const i_isInClip = false)
        : m_rRegistry(i_rReg)
        , m_bInClipboard(i_isInClip), m_bInUndo(false), m_bInContent(true) {}
    bool m_bInClipboard;
    bool m_bInUndo;
    bool m_bInContent;
    virtual bool IsInClipboard() const { return m_bInClipboard; }
    virtual bool IsInUndo() const { return m_bInUndo; }
    virtual bool IsInContent() const { return m_bInContent; }
    virtual ::sfx2::IXmlIdRegistry& GetRegistry() { return m_rRegistry; }
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::rdf::XMetadatable > MakeUnoObject() { return 0; }
};

static bool operator==(beans::StringPair p1, beans::StringPair p2)
{
    return p1.First == p2.First && p1.Second == p2.Second;
}

TEST_F(MetadatableTest, test)
{
    OSL_TRACE("SwMetadatable test(): start\n");
    ::std::auto_ptr< ::sfx2::IXmlIdRegistry > const pReg(
        ::sfx2::createXmlIdRegistry(false) );
    ::std::auto_ptr< ::sfx2::IXmlIdRegistry > const pRegClip(
        ::sfx2::createXmlIdRegistry(true) );

    MockMetadatable m1(*pReg);
    MockMetadatable m2(*pReg);
    MockMetadatable m3(*pReg);
    MockMetadatable m4(*pReg);
    MockMetadatable m5(*pReg);
    ::rtl::OUString empty;
    ::rtl::OUString content( ::rtl::OUString::createFromAscii("content.xml") );
    ::rtl::OUString styles ( ::rtl::OUString::createFromAscii("styles.xml") );
    ::rtl::OUString sid1( ::rtl::OUString::createFromAscii("id1") );
    ::rtl::OUString sid2( ::rtl::OUString::createFromAscii("id2") );
    ::rtl::OUString sid3( ::rtl::OUString::createFromAscii("id3") );
    ::rtl::OUString sid4( ::rtl::OUString::createFromAscii("id4") );
    beans::StringPair id1(content, sid1);
    beans::StringPair id2(content, sid2);
    beans::StringPair id3(content, sid3);
    beans::StringPair id4(styles,  sid4);
    beans::StringPair id3e(empty,  sid3);
    beans::StringPair id4e(empty,  sid4);
    m1.SetMetadataReference(id1);
    ASSERT_TRUE(m1.GetMetadataReference() == id1) << "set failed";
    try {
        m2.SetMetadataReference(id1);
        ASSERT_TRUE(false) << "set duplicate succeeded";
    } catch (lang::IllegalArgumentException) { }
    m1.SetMetadataReference(id1);
    ASSERT_TRUE(m1.GetMetadataReference() == id1) << "set failed (existing)";
    m1.EnsureMetadataReference();
    ASSERT_TRUE(m1.GetMetadataReference() == id1) << "ensure failed (existing)";

    m2.EnsureMetadataReference();
    beans::StringPair m2id(m2.GetMetadataReference());
    ASSERT_TRUE(m2id.Second.getLength()) << "ensure failed";
    m2.EnsureMetadataReference();
    ASSERT_TRUE(m2.GetMetadataReference() == m2id) << "ensure failed (idempotent)";

    m1.m_bInUndo = true;
    ASSERT_TRUE(!m1.GetMetadataReference().Second.getLength()) << "move to undo failed";

    m1.m_bInUndo = false;
    ASSERT_TRUE(m1.GetMetadataReference() == id1) << "move from undo failed";

    m1.m_bInUndo = true;
    try {
        m2.SetMetadataReference(id1); // steal!
    } catch (lang::IllegalArgumentException &) {
        FAIL() << "set duplicate to undo failed";
    }
    m1.m_bInUndo = false;
    ASSERT_TRUE(!m1.GetMetadataReference().Second.getLength()) << "move from undo: duplicate";

    m3.RegisterAsCopyOf(m2);
    ASSERT_TRUE(m2.GetMetadataReference() == id1) << "copy: source";
    ASSERT_TRUE(!m3.GetMetadataReference().Second.getLength()) << "copy: duplicate";
    m4.RegisterAsCopyOf(m3);
    ASSERT_TRUE(m2.GetMetadataReference() == id1) << "copy: source";
    ASSERT_TRUE(!m3.GetMetadataReference().Second.getLength()) << "copy: duplicate";
    ASSERT_TRUE(!m4.GetMetadataReference().Second.getLength()) << "copy: duplicate";
    m2.m_bInUndo = true;
    ASSERT_TRUE(m3.GetMetadataReference() == id1) << "duplicate to undo";
    ASSERT_TRUE(!m2.GetMetadataReference().Second.getLength()) << "duplicate to undo";
    m2.m_bInUndo = false;
    ASSERT_TRUE(m2.GetMetadataReference() == id1) << "duplicate from undo";
    ASSERT_TRUE(!m3.GetMetadataReference().Second.getLength()) << "duplicate from undo";

    m4.EnsureMetadataReference(); // new!
    beans::StringPair m4id(m4.GetMetadataReference());
    ASSERT_TRUE(m4id.Second.getLength() && !(m4id == id1)) << "ensure on duplicate";

    MockMetadatable mc1(*pRegClip, true); // in clipboard
    MockMetadatable mc2(*pRegClip, true);
    MockMetadatable mc3(*pRegClip, true);
    MockMetadatable mc4(*pRegClip, true);
    MockMetadatable m2p(*pReg);
    MockMetadatable m3p(*pReg);

    mc1.SetMetadataReference(id2);
    ASSERT_TRUE(mc1.GetMetadataReference() == id2) << "set failed";
    try {
        mc2.SetMetadataReference(id2);
        FAIL() << "set duplicate succeeded";
    } catch (lang::IllegalArgumentException) { }
    mc1.SetMetadataReference(id2);
    ASSERT_TRUE(mc1.GetMetadataReference() == id2) << "set failed (existing)";
    mc1.EnsureMetadataReference();
    ASSERT_TRUE(mc1.GetMetadataReference() == id2) << "ensure failed (existing)";
    mc2.EnsureMetadataReference();
    beans::StringPair mc2id(mc2.GetMetadataReference());
    ASSERT_TRUE(mc2id.Second.getLength()) << "ensure failed";
    mc2.EnsureMetadataReference();
    ASSERT_TRUE(mc2.GetMetadataReference() == mc2id) << "ensure failed (idempotent)";
    mc2.RemoveMetadataReference();
    ASSERT_TRUE(!mc2.GetMetadataReference().Second.getLength()) << "remove failed";

    // set up mc2 as copy of m2 and mc3 as copy of m3
    mc3.RegisterAsCopyOf(m3);
    ASSERT_TRUE(!mc3.GetMetadataReference().Second.getLength()) << "copy to clipboard (latent)";
    mc2.RegisterAsCopyOf(m2);
    ASSERT_TRUE(mc2.GetMetadataReference() == id1) << "copy to clipboard (non-latent)";
    // paste mc2 to m2p and mc3 to m3p
    m2p.RegisterAsCopyOf(mc2);
    ASSERT_TRUE(!m2p.GetMetadataReference().Second.getLength()) << "paste from clipboard (non-latent)";
    m3p.RegisterAsCopyOf(mc3);
    ASSERT_TRUE(!m3p.GetMetadataReference().Second.getLength()) << "paste from clipboard (latent)";
    // delete m2, m2p, m3
    m2.RemoveMetadataReference();
    ASSERT_TRUE(!m2.GetMetadataReference().Second.getLength()) << "remove failed";
    ASSERT_TRUE(m2p.GetMetadataReference() == id1) << "paste-remove (non-latent)";
    m2p.RemoveMetadataReference();
    ASSERT_TRUE(!m2p.GetMetadataReference().Second.getLength()) << "remove failed";
    ASSERT_TRUE(m3.GetMetadataReference() == id1) << "paste-remove2 (non-latent)";
    m3.RemoveMetadataReference();
    ASSERT_TRUE(!m3.GetMetadataReference().Second.getLength()) << "remove failed";
    ASSERT_TRUE(m3p.GetMetadataReference() == id1) << "paste-remove (latent)";
    // delete mc2
    mc2.SetMetadataReference(beans::StringPair());
    ASSERT_TRUE(!mc3.GetMetadataReference().Second.getLength()) << "in clipboard becomes non-latent";
    // paste mc2
    m2p.RegisterAsCopyOf(mc2);
    ASSERT_TRUE(!m2p.GetMetadataReference().Second.getLength()) << "remove-paste";
    ASSERT_TRUE(m3p.GetMetadataReference() == id1) << "remove-paste (stolen)";

    // auto-detect stream
    m5.SetMetadataReference(id3e);
    ASSERT_TRUE(m5.GetMetadataReference() == id3) << "auto-detect (content)";
    m5.m_bInContent = false;
    m5.SetMetadataReference(id4e);
    ASSERT_TRUE(m5.GetMetadataReference() == id4) << "auto-detect (styles)";

    OSL_TRACE("sfx2::Metadatable test(): finished\n");
}


}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

