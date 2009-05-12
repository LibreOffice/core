/*
 * MultiStratumBackend.java
 *
 * Created on 24. March 2004, 13:48
 */

package mod._cfgmgr2;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XStringSubstitution;
import java.io.PrintWriter;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

public class MultiStratumBackend extends TestCase {

    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        XInterface oObj = null;

        Object[] args = new Object[1];
        String AdminURL = "";

        try {
            XStringSubstitution sts = createStringSubstitution(
                                              (XMultiServiceFactory) tParam.getMSF());
                        AdminURL = sts.getSubstituteVariableValue("$(inst)") +
                       "/share/registry";
            args[0] = ((XMultiServiceFactory)tParam.getMSF())
                                      .createInstance("com.sun.star.configuration.bootstrap.BootstrapContext");
            oObj = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                      .createInstanceWithArguments("com.sun.star.comp.configuration.backend.MultiStratumBackend",args);

        } catch (com.sun.star.uno.Exception e) {
        }

        log.println("Implementation name: "+ util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        //objRelation for XInitialization
        tEnv.addObjRelation("XInitialization.args", args);

        //objRelation for XBackendEntities
        tEnv.addObjRelation("NoAdmin", AdminURL);
        tEnv.addObjRelation("MSB", Boolean.TRUE);

        return tEnv;
    }

    public static XStringSubstitution createStringSubstitution(XMultiServiceFactory xMSF) {
        Object xPathSubst = null;

        try {
            xPathSubst = xMSF.createInstance(
                                 "com.sun.star.util.PathSubstitution");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }

        if (xPathSubst != null) {
            return (XStringSubstitution) UnoRuntime.queryInterface(
                           XStringSubstitution.class, xPathSubst);
        } else {
            return null;
        }
    }
}
