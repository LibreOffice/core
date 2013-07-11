package org.libreoffice.impressremote;

import android.os.Bundle;
import android.support.v4.app.FragmentTransaction;

import com.actionbarsherlock.app.SherlockFragmentActivity;
import org.libreoffice.impressremote.communication.Server;

public class ComputerConnectionActivity extends SherlockFragmentActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setUpComputerConnectionFragment(extractReceivedComputer());
    }

    private void setUpComputerConnectionFragment(Server aComputer) {
        FragmentTransaction aTransaction = getSupportFragmentManager().beginTransaction();

        aTransaction.replace(android.R.id.content, ComputerConnectionFragment.newInstance(aComputer));

        aTransaction.commit();
    }

    private Server extractReceivedComputer() {
        return getIntent().getParcelableExtra("COMPUTER");
    }
}
