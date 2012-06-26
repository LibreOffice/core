/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

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
