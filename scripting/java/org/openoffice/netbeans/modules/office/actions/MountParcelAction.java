package org.openoffice.netbeans.modules.office.actions;

import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.RequestProcessor;
import org.openide.util.actions.CookieAction;

/**
 *
 * @author  adams
 * @version 1.0
 */
public class MountParcelAction extends CookieAction
{
    public MountParcelAction()
    {
    }

    public java.lang.String getName()
    {
        return "Mount Parcel"; //NOI18N
    }

    public HelpCtx getHelpCtx()
    {
        return HelpCtx.DEFAULT_HELP;
    }

    protected int mode()
    {
        // enable duplication for as many qualifying nodes as are selected:
        return CookieAction.MODE_ALL;
    }

    protected java.lang.Class[] cookieClasses()
    {
        return new Class[] {ParcelCookie.class};
    }

    protected void performAction(final Node[] activatedNodes)
    {
        RequestProcessor.getDefault().post(new Runnable()
        {
            public void run()
            {
                for (int i=0; i<activatedNodes.length; i++)
                {
                    ParcelCookie mpc = (ParcelCookie)activatedNodes[i].getCookie(ParcelCookie.class);
                    if (mpc != null)
                    {
                        mpc.mount();
                    }
                }
            }
        });
    }
}
