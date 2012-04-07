package org.libreoffice.experimental.eagles051387;

import org.libreoffice.experimentalUI.eagles051387.R;
import android.app.Activity;
import android.os.Bundle;

public class Main_Menu extends Activity
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        setContentView(R.layout.star_center_menu_list);
    }
}