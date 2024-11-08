/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/bootstrapfixturebase.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>

using namespace ::com::sun::star;

class ZipPackageTest : public test::BootstrapFixtureBase
{
};

auto const ZipPackage(u"com.sun.star.packages.comp.ZipPackage"_ustr);

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testDuplicate)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/duplicate-files.odt"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };

    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testUnicodeODT)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/unicode-path.odt"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testUnicodeDOCX)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/unicode-path.docx"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testAbsolutePathODT)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/slash.odt"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testDotPathDOCX)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/dot-slash.docx"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testConcatODT)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/two-zips.odt"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testConcatDOCX)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/two-zips.docx"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testCaseInsensitiveDOCX)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/casing.docx"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::OFOPXML)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testOverlapDOCX)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/overlap.docx"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testInnerGapDOCX)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/inner-gap.docx"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testDataDescriptorDeflatedDOCX)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/dd-deflated.docx"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testDataDescriptorStoredDOCX)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/dd-stored.docx"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::OFOPXML)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::OFOPXML)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testZip64End)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/zip64-eocd.docx"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testTdf163364)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/tdf163364.ods"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::PACKAGE)))
    };

    // don't load corrupted zip file
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    try
    {
        uno::Sequence<uno::Any> const args2{
            uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
            uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
        };
        m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                               m_xContext);
    }
    catch (css::packages::zip::ZipIOException const&)
    {
        // check that this doesn't crash, it doesn't matter if it succeeds or not
    }
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testTdf163341)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/tdf163341.ods"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::PACKAGE)))
    };

    // this Zip64 should load successfully
    m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args,
                                                                           m_xContext);
}

CPPUNIT_TEST_FIXTURE(ZipPackageTest, testTdf163818)
{
    auto const url(m_directories.getURLFromSrc(u"/package/qa/cppunit/data/tdf163818.odg"));
    uno::Sequence<uno::Any> const args{
        uno::Any(url),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::PACKAGE)))
    };

    // unclear if this should be allowed?
    CPPUNIT_ASSERT_THROW(m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                             ZipPackage, args, m_xContext),
                         css::packages::zip::ZipIOException);

    // recovery should work
    uno::Sequence<uno::Any> const args2{
        uno::Any(url), uno::Any(beans::NamedValue(u"RepairPackage"_ustr, uno::Any(true))),
        uno::Any(beans::NamedValue("StorageFormat", uno::Any(embed::StorageFormats::ZIP)))
    };
    m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(ZipPackage, args2,
                                                                           m_xContext);
}

//CPPUNIT_TEST_SUITE_REGISTRATION(...);
//CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
