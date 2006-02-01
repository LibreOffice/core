/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleRelationSet.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-02-01 14:19:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package complex.accessibility;

import com.sun.star.accessibility.AccessibleRelation;
import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleRelationSet;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XModel;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import complexlib.ComplexTestCase;
import java.io.PrintWriter;
import lib.StatusException;
import util.AccessibilityTools;
import util.WriterTools;
import util.utils;

public class AccessibleRelationSet extends ComplexTestCase {

    private static XAccessible para1 = null;
    private static XAccessible para2 = null;
    private static XAccessible para3 = null;
    private static XTextDocument xTextDoc = null;
    private final static String[] types = {"INVALID","CONTENT_FLOWS_FROM","CONTENT_FLOWS_TO","CONTROLLED_BY","CONTROLLER_FOR","LABEL_FOR","LABELED_BY","MEMBER_OF","SUB_WINDOW_OF"};

    public String[] getTestMethodNames() {
        return new String[]{"contents_flows_to","contents_flows_from"};
    }

    public void contents_flows_to() {
        XAccessibleRelationSet set = getAccessibleRelation(para1);

        boolean res = true;
        short firstrelation=-1;
        XAccessibleText atarget=null;
        if (set != null) {
            log.println("Count of relations "+set.getRelationCount());
            assure("didn't gain correct count of relations",
                   set.getRelationCount() == 1);
            try {
                firstrelation = set.getRelation(0).RelationType;
                Object oTmp = set.getRelation(0).TargetSet[0];
                atarget = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
            } catch (IndexOutOfBoundsException e) {
                log.println("Exception when getting relations "+e);
                res = false;
            }
        }

        log.println("Expected for paragraph 0 "+types[2]);
        log.println("gained for paragraph 0 "+types[firstrelation]);
        res = types[2].equals(types[firstrelation]);
        assure("didn't gain correct relation type",res);

        log.println("Text of target paragraph "+atarget.getText());
        XAccessibleText paraTxt2 =
          (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, para2);
        assure("didn't gain correct target paragraph",
               atarget.getText().equals(paraTxt2.getText()) );
    }

    public void contents_flows_from() {
        XAccessibleRelationSet set = getAccessibleRelation(para2);

        boolean res = true;
        short[] relationtypes = new short[2];
        XAccessibleText[] atargets = new XAccessibleText[2];
        if (set != null) {
            log.println("Count of relations "+set.getRelationCount());
            assure("didn't gain correct count of relations",
                   set.getRelationCount() == 2);
            try {
                short tmprelation = set.getRelation(0).RelationType;
                if ( tmprelation == 1 )
                {
                  Object oTmp = set.getRelation(0).TargetSet[0];
                  atargets[0] = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
                  relationtypes[0] = tmprelation;
                }
                else if ( tmprelation == 2 )
                {
                  Object oTmp = set.getRelation(0).TargetSet[0];
                  atargets[1] = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
                  relationtypes[1] = tmprelation;
                }
                else
                {
                  assure("didn't gain correct relation type", false);
                }
                tmprelation = set.getRelation(1).RelationType;
                if ( tmprelation == 1 )
                {
                  Object oTmp = set.getRelation(1).TargetSet[0];
                  atargets[0] = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
                  relationtypes[0] = tmprelation;
                }
                else if ( tmprelation == 2 )
                {
                  Object oTmp = set.getRelation(1).TargetSet[0];
                  atargets[1] = (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, oTmp);
                  relationtypes[1] = tmprelation;
                }
                else
                {
                  assure("didn't gain correct relation type", false);
                }
            } catch (IndexOutOfBoundsException e) {
                log.println("Exception when getting relations "+e);
                res = false;
            }
        }

        log.println("### Checking "+types[1]+" for paragraph 1");
        log.println("Expected for paragraph 1 "+types[1]);
        log.println("gained for paragraph 1 "+types[relationtypes[0]]);
        res = types[1].equals(types[relationtypes[0]]);
        assure("didn't gain correct relation type",res);

        log.println("Text of target paragraph "+atargets[0].getText());
        XAccessibleText paraTxt1 =
          (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, para1);
        assure("didn't gain correct target paragraph",
               atargets[0].getText().equals(paraTxt1.getText()) );

        log.println("### Checking "+types[2]+" for paragraph 1");
        log.println("Expected for paragraph 1 "+types[2]);
        log.println("gained for paragraph 1 "+types[relationtypes[1]]);
        res = types[2].equals(types[relationtypes[1]]);
        assure("didn't gain correct relation type",res);

        log.println("Text of target paragraph "+atargets[1].getText());
        XAccessibleText paraTxt3 =
          (XAccessibleText) UnoRuntime.queryInterface(XAccessibleText.class, para3);
        assure("didn't gain correct target paragraph",
               atargets[1].getText().equals(paraTxt3.getText()) );
    }

    private boolean getResult(XAccessible aPara, short index, int nr) {
        XAccessibleRelationSet set = getAccessibleRelation(aPara);

        boolean res = true;
        short firstrelation=-1;
        if (set != null) {
            log.println("Count of relations "+set.getRelationCount());
            try {
                firstrelation = set.getRelation(0).RelationType;
            } catch (IndexOutOfBoundsException e) {
                log.println("Exception when getting relations "+e);
                res = false;
            }
        }


        log.println("Expected for paragraph "+nr+" "+types[index]);
        log.println("gained for paragraph "+nr+" "+types[firstrelation]);
        res = types[index].equals(types[firstrelation]);
        return res;
    }


    public void before() {
        log.println( "creating a text document" );
        xTextDoc = WriterTools.createTextDoc( (XMultiServiceFactory) param.getMSF());

        XText oText = xTextDoc.getText();
        XTextCursor oCursor = oText.createTextCursor();

        log.println( "inserting some lines" );
        try {
            for (int i=0; i<5; i++){
                oText.insertString( oCursor,"Paragraph Number: " + i, false);
                oText.insertControlCharacter(
                        oCursor, ControlCharacter.PARAGRAPH_BREAK, false );
            }
        } catch ( com.sun.star.lang.IllegalArgumentException e ){
            e.printStackTrace((PrintWriter)log);
            throw new StatusException( "Couldn't insert lines", e );
        }

        XModel aModel = (XModel)
        UnoRuntime.queryInterface(XModel.class, xTextDoc);

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow( (XMultiServiceFactory) param.getMSF(), aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.getAccessibleObjectForRole(xRoot, AccessibleRole.DOCUMENT);

        try {
            para1 = at.SearchedContext.getAccessibleChild(0);
            para2 = at.SearchedContext.getAccessibleChild(1);
            para3 = at.SearchedContext.getAccessibleChild(2);
        } catch(IndexOutOfBoundsException e) {
            e.printStackTrace((PrintWriter)log);
            throw new StatusException( "Couldn't insert lines", e );
        }

        log.println("ImplementationName (para1)" + utils.getImplName(para1));
        log.println("ImplementationName (para2)" + utils.getImplName(para2));
    }

    public void after() {
        log.println("close text document");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    public XAccessibleRelationSet getAccessibleRelation(XAccessible xAcc) {
        XAccessibleContext oObj = (XAccessibleContext)
        UnoRuntime.queryInterface(XAccessibleContext.class, xAcc);

        XAccessibleRelationSet set = oObj.getAccessibleRelationSet();
        return set;
    }


}
