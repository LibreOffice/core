package org.libreoffice.impressremote;

import android.os.Bundle;
import android.support.v4.app.FragmentTransaction;

import com.actionbarsherlock.app.SherlockFragmentActivity;
import com.actionbarsherlock.view.MenuItem;
import org.libreoffice.impressremote.communication.Server;

public class ComputerConnectionActivity extends SherlockFragmentActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setUpHomeButton();

        setUpComputerConnectionFragment(extractReceivedComputer());
    }

    private void setUpHomeButton() {
        getSupportActionBar().setHomeButtonEnabled(true);
    }

    private void setUpComputerConnectionFragment(Server aComputer) {
        FragmentTransaction aTransaction = getSupportFragmentManager().beginTransaction();

        aTransaction.replace(android.R.id.content, ComputerConnectionFragment.newInstance(aComputer));

        aTransaction.commit();
    }

    private Server extractReceivedComputer() {
        return getIntent().getParcelableExtra(Intents.Extras.SERVER);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case android.R.id.home:
                navigateUp();
                return true;

            default:
                return super.onOptionsItemSelected(aMenuItem);
        }
    }

    private void navigateUp() {
        finish();
    }
}
