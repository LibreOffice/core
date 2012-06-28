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



package testsuite;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

import testcase.sd.headerandfooter.*;

@RunWith(Suite.class)
@SuiteClasses({CopySlideWithApplyFooter.class,
    CopySlideWithApplyToAllFooter.class,
    CopySlideWithNotesHeaderFooter.class,
    DuplicateSlideWithApplyToAllFooter.class,
    FooterNotShowOn1stSlide.class,
    InsertApplyFooterOnSlide.class,
    InsertApplyToAllFooterOnNotes.class,
    InsertApplyToAllFooterOnSlide.class,
    InsertAutoUpdateTimeOnNotes.class,
    InsertAutoUpdateTimeFooter.class,
    InsertHeaderFooterOnNotes.class,
    InsertHeaderFooterOnSlide.class,
    OpenAOO34WithHeaderFooter.class,
    OpenPPTWithHeaderFooter.class
    })
public class FVT {

}
