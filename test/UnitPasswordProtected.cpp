/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Unit test for password-protected document handling.
 */

#include <config.h>

#include <UnitWSDClient.hpp>

class UnitPasswordProtectedDocWithoutPassword : public UnitWSDClient
{
    STATE_ENUM(Phase, Load, WaitError) _phase;
    bool _gotPasswordRequired;

public:
    UnitPasswordProtectedDocWithoutPassword()
        : UnitWSDClient("UnitPasswordProtectedDocWithoutPassword")
        , _phase(Phase::Load)
        , _gotPasswordRequired(false)
    {
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');

        if (!_gotPasswordRequired)
        {
            LOK_ASSERT_EQUAL_STR("error: cmd=load kind=passwordrequired:to-view", message);
            _gotPasswordRequired = true;
        }
        else
        {
            // We get faileddocloading followed by docunloading errors.
            LOK_ASSERT(message == "error: cmd=load kind=faileddocloading" ||
                       message == "error: cmd=load kind=docunloading");
        }

        passTest("Password is required for viewing");
        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitError);
                connectAndLoadLocalDocument("password-protected.ods");
                break;
            }
            case Phase::WaitError:
                break;
        }
    }
};

class UnitPasswordProtectedDocWrongPassword : public UnitWSDClient
{
    STATE_ENUM(Phase, Load, WaitError) _phase;
    bool _gotWrongPassword;

public:
    UnitPasswordProtectedDocWrongPassword()
        : UnitWSDClient("UnitPasswordProtectedDocWrongPassword")
        , _phase(Phase::Load)
        , _gotWrongPassword(false)
    {
    }

    bool onDocumentError(const std::string& message) override
    {
        TST_LOG("onDocumentError: [" << message << ']');
        if (!_gotWrongPassword)
        {
            LOK_ASSERT_EQUAL_STR("error: cmd=load kind=wrongpassword", message);
            _gotWrongPassword = true;
        }
        else
        {
            // We get faileddocloading followed by docunloading errors.
            LOK_ASSERT(message == "error: cmd=load kind=faileddocloading" ||
                       message == "error: cmd=load kind=docunloading");
        }

        passTest("Password is required for viewing");
        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitError);

                const std::string docFilename = "password-protected.ods";
                const std::string documentURL = connectToLocalDocument(docFilename);

                TST_LOG("Loading local document [" << docFilename << "] with URL: " << documentURL);
                WSD_CMD("load url=" + documentURL + " password=2");
                break;
            }
            case Phase::WaitError:
                break;
        }
    }
};

class UnitPasswordProtectedDocCorrectPassword : public UnitWSDClient
{
    STATE_ENUM(Phase, Load, WaitLoad) _phase;

public:
    UnitPasswordProtectedDocCorrectPassword()
        : UnitWSDClient("UnitPasswordProtectedDocCorrectPassword")
        , _phase(Phase::Load)
    {
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoad);

        passTest("Loaded successfully");
        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoad);

                const std::string docFilename = "password-protected.ods";
                const std::string documentURL = connectToLocalDocument(docFilename);

                TST_LOG("Loading local document [" << docFilename << "] with URL: " << documentURL);
                WSD_CMD("load url=" + documentURL + " password=1");
                break;
            }
            case Phase::WaitLoad:
                break;
        }
    }
};

class UnitPasswordProtectedOOXMLDoc : public UnitWSDClient
{
    STATE_ENUM(Phase, Load, WaitLoad) _phase;

public:
    UnitPasswordProtectedOOXMLDoc()
        : UnitWSDClient("UnitPasswordProtectedOOXMLDoc")
        , _phase(Phase::Load)
    {
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoad);

        passTest("Loaded successfully");
        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoad);

                const std::string docFilename = "password-protected.docx";
                const std::string documentURL = connectToLocalDocument(docFilename);

                TST_LOG("Loading local document [" << docFilename << "] with URL: " << documentURL);
                WSD_CMD("load url=" + documentURL + " password=abc");
                break;
            }
            case Phase::WaitLoad:
                break;
        }
    }
};

class UnitPasswordProtectedBinMSODoc : public UnitWSDClient
{
    STATE_ENUM(Phase, Load, WaitLoad) _phase;

public:
    UnitPasswordProtectedBinMSODoc()
        : UnitWSDClient("UnitPasswordProtectedBinMSODoc")
        , _phase(Phase::Load)
    {
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitLoad);

        passTest("Loaded successfully");
        return true;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitLoad);

                const std::string docFilename = "password-protected.doc";
                const std::string documentURL = connectToLocalDocument(docFilename);

                TST_LOG("Loading local document [" << docFilename << "] with URL: " << documentURL);
                WSD_CMD("load url=" + documentURL + " password=abc");
                break;
            }
            case Phase::WaitLoad:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    return new UnitBase* [6]
    {
        new UnitPasswordProtectedDocWithoutPassword(), new UnitPasswordProtectedDocWrongPassword(),
            new UnitPasswordProtectedDocCorrectPassword(), new UnitPasswordProtectedOOXMLDoc(),
            new UnitPasswordProtectedBinMSODoc(), nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
