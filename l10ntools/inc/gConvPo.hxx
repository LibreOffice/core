/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef GCONPO_HXX
#define GCONPO_HXX
#include "gConv.hxx"



class convert_po : public convert_gen
{
    public:
        bool mbExpectId;
        bool mbExpectStr;


        convert_po(l10nMem& crMemory);
        ~convert_po();

        void startLook ();
        void setValue  (char *syyText, int iLineCnt);
        void setFuzzy  ();
        void setKey    (char *syyText);
        void setMsgId  ();
        void setMsgStr ();
        void handleNL  ();

    private:
        std::string  msId;
        std::string  msStr;
        std::string  msKey;
        bool         mbFuzzy;
        std::filebuf outBuffer;

        void execute() override;

        void startSave(const std::string& sLanguage,
                       const std::string& sFile) override;
        void save(const std::string& sFileName,
                  const std::string& sKey,
                  const std::string& sENUStext,
                  const std::string& sText,
                  bool               bFuzzy) override;
        void endSave() override;
};
#endif
