package org.libreoffice.impressremote.communication;

/**
 * Interface to send commands to the server.
 *
 * @author Andrzej J.R. Hunt
 */
public class Transmitter {

	private Client mClient;

	public Transmitter(Client aClient) {
		mClient = aClient;
	}

	public void nextTransition() {
		mClient.sendCommand("transition_next\n\n");
	}

	public void previousTransition() {

		mClient.sendCommand("transition_previous\n\n");
	}

	public void gotoSlide(int slide) {
		mClient.sendCommand("goto_slide\n" + slide + "\n\n");
	}

}
