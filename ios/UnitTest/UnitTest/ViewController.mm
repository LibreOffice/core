/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <iostream>

#include <sal/config.h>

#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/uno/Any.hxx>

#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestProgressListener.h>

#import <LibreOfficeKit/LibreOfficeKit.hxx>

extern "C" {
#import <native-code.h>
}

#include <premac.h>
#import <CoreGraphics/CoreGraphics.h>
#import "ViewController.h"
#include <postmac.h>

// This is from online's Mobile app (as it is called at the moment);
// should of course be factored out to some common place. Here in
// core?

static thread_local CGContextRef cgc = nullptr;

static unsigned char *lo_ios_app_get_cgcontext_for_buffer(unsigned char *buffer, int width, int height)
{
    assert(cgc == nullptr);

    cgc = CGBitmapContextCreate(buffer, width, height, 8, width*4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaNoneSkipFirst);

    CGContextTranslateCTM(cgc, 0, height);
    CGContextScaleCTM(cgc, 1, -1);

    return (unsigned char*)cgc;
}

static void lo_ios_app_release_cgcontext_for_buffer()
{
    assert(cgc != nullptr);
    CGContextRelease(cgc);
    cgc = nullptr;
}

@interface ViewController ()

@end

#define CPPUNIT_PLUGIN_EXPORTED_NAME CppuMisc
#include "../../../cppuhelper/qa/misc/test_misc.cxx"

#undef CPPUNIT_PLUGIN_EXPORTED_NAME
#define CPPUNIT_PLUGIN_EXPORTED_NAME CppuUnourl
#include "../../../cppuhelper/qa/unourl/cppu_unourl.cxx"

#define main tilebench_main
#include "../../../libreofficekit/qa/tilebench/tilebench.cxx"

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // First run some normal cppunit tests

    CppUnit::TestResult result;

    CppUnit::TextTestProgressListener logger;
    result.addListener(&logger);

    CppUnit::TestResultCollector collector;
    result.addListener(&collector);

    CppUnit::TestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    runner.run(result);

    CppUnit::CompilerOutputter outputter(&collector, std::cerr);
    outputter.write();

    // Then some more specific stuff
    tilebench_main(0, nullptr);
}

@end
