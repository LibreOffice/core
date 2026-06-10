# Collabora Online and Collabora Office

> [!IMPORTANT]
> **The CollaboraOnline/online.mirror repository on GitHub is a read-only mirror.**
> Collabora Online and Collabora Office are developed on our Gerrit
> instance at **https://gerrit.collaboraoffice.com**; the GitHub repository only mirrors that
> source code.
>
> - **Code contributions** go through Gerrit, not GitHub. Pull requests are disabled here; see
>   [CONTRIBUTING.md](CONTRIBUTING.md#contributing-to-source-code) for how to submit a patch.
> - **Report issues** at the [CollaboraOnline/online issue tracker](https://github.com/CollaboraOnline/online/issues/new/choose).

[![Development: Gerrit](https://img.shields.io/badge/Development-Gerrit-orange?logo=git "Code development and review happen on Gerrit. GitHub pull requests are disabled.")](https://gerrit.collaboraoffice.com)


[![Matrix](https://img.shields.io/badge/Matrix-%23cool--dev-turquoise.svg)](https://matrix.to/#/#cool-dev:matrix.org)
[![Telegram](https://img.shields.io/badge/Telegram-Collabora%20Online-green.svg)](https://t.me/CollaboraOnline)
[![Forum](https://img.shields.io/badge/Forum-Discourse-blue.svg)](https://forum.collaboraonline.com/)
[![Website](https://img.shields.io/badge/Website-www.collaboraoffice.org-blueviolet.svg)](https://www.collaboraoffice.org/)
[![L10n](https://img.shields.io/badge/L10n-Weblate-lightgrey.svg)](https://hosted.weblate.org/projects/collabora-online/)


**Secure Document Collaboration. Controlled by You.**

![](https://www.collaboraonline.com/wp-content/uploads/2024/02/Website_COOL_2305_Home-1024x366.png)

## Learn more 💡
* [Collabora Online Development Edition (CODE)](https://www.collaboraonline.com/code/)
* [Collabora Online](https://www.collaboraonline.com/collabora-online/)
* [Collabora Office](https://www.collaboraonline.com/collabora-office/)
* [Collabora Office for iOS and Android](https://www.collaboraonline.com/collabora-office-android-ios/)
* For additional details, build instructions, downloads, and more, please visit [www.collaboraoffice.org](https://www.collaboraoffice.org/). You can also explore the [SDK Documentation](https://sdk.collaboraonline.com/).

## Key features
* View and edit text documents, spreadsheets, presentations & more
* Collaborative editing features
* Collabora Online works in any modern browser – no plugin needed
* [Integrate, extend and customise](https://www.collaboraonline.com/integrations/)
* Open Source – primarily under the [MPLv2](http://mozilla.org/MPL/2.0/) license. Some parts are under other open source licences, see e.g. [browser/LICENSE](https://github.com/CollaboraOnline/online/blob/main/browser/LICENSE).

## Get in touch 💬

* [📋 Forum](https://forum.collaboraonline.com/)
* [👥 Facebook](https://www.facebook.com/collaboraoffice/)
* [🅾 Instagram](https://www.instagram.com/collaboraoffice/)
* [🐣 𝕏(Twitter)](https://twitter.com/CollaboraOffice)
* [🦋 Bluesky](https://bsky.app/profile/collaboraonline.com)
* [🐘 Mastodon](https://mastodon.social/@CollaboraOffice)
* [💼 LinkedIn](https://www.linkedin.com/products/collaboraproductivity-collabora-online/)
* [🎥 YouTube](https://www.youtube.com/@CollaboraOnline)

## Integrating

To [integrate](https://sdk.collaboraonline.com/docs/why_integrate.html) Collabora Online into your own solution, you can see our [step-by-step tutorial](https://sdk.collaboraonline.com/docs/Step_by_step_tutorial.html), view the [available integrations](https://sdk.collaboraonline.com/docs/available_integrations.html), understand the [post message API](https://sdk.collaboraonline.com/docs/postmessage_api.html) used to customize and interact with Collabora Online, and a lot more.


### Test running with integration for developers

Set up Nextcloud or ownCloud on localhost and install the richdocuments app, or explore other integrations that can be used to test Collabora Online at https://www.collaboraonline.com/integrations/.

Good tutorials exist how to install ownCloud or Nextcloud, we don't repeat
them here. richdocuments is called Collabora Online in the respective app
stores / marketplaces / whatever.

When you have a running Nextcloud or ownCloud instance at
http://localhost/nextcloud or at http://localhost/owncloud
go to Collabora Online settings, and set the WOPI URL to
http://localhost:9980

Then in the build tree, edit the generated coolwsd.xml and set ssl setting to
false. You can run make run, and test coolwsd with the ownCloud or Nextcloud
integration.

Note: if SSL is enabled in either Online or the integration, both must
have SSL enabled. That is, you must access NC/OC using https:// as well
as configure the Collabora Online endpoint in NC/OC as https://localhost:9980.

## Find a partner

Do you want a ready-to-go or simplified solution? Find a trusted vendor from our [list of global partners](https://www.collaboraonline.com/partners/) who integrate and host Collabora Online.

## Developer assistance
Please ask your questions on any of the bridged Matrix/Telegram rooms
* Matrix: [#cool-dev:matrix.org](https://matrix.to/#/#cool-dev:matrix.org)
* Telegram: [CollaboraOnline](https://t.me/CollaboraOnline)

Join the conversation on our Discourse server at https://forum.collaboraonline.com/

Watch the tinderbox status (it should be green):

[![tinderbox status](https://cpci.cbg.collabora.co.uk:8080/buildStatus/icon?job=Tinderbox+for+online+main&subject=tinderbox)](https://cpci.cbg.collabora.co.uk:8080/view/Tinderbox/job/Tinderbox%20for%20online%20main/)

## Development bits

This project has several components:
* **engine/**
  * The document rendering engine
* **wsd/**
  * The Web Services Daemon - which accepts external connections
* **kit/**
  * The client which lives in its own chroot and renders documents
* **common/**
  * Shared code between these processes
* **browser/**
  * The client side JavaScript component
* **test/**
  * C++ based unit tests
* **cypress_test/**
  * JavaScript based integration tests

## Further recommended reading with build details

* **[Build Collabora Online](https://www.collaboraoffice.org/post/build-code/)**
* **[Build Collabora Office for iOS](https://www.collaboraoffice.org/post/build-code-ios/)**
* **[Build Collabora Office for Android](https://www.collaboraoffice.org/post/build-code-android/)**
* **[Build Collabora Office for Windows](https://www.collaboraoffice.org/post/build-co-windows/)**
* **[Build Collabora Office for macOS](https://www.collaboraoffice.org/post/build-co-mac/)**
* **[Build Collabora Office for Linux](https://www.collaboraoffice.org/post/build-co-linux/)**

## Admin Panel

You can access the admin panel by directly accessing the admin.html file
from browser directory.

Websocket connections to admin console can be made at path: /adminws/ on the
same url and port as coolwsd is running on. However, one needs a JWT token to
authenticate to the admin console websocket. This is stored as a cookie with
`Path: /adminws/` when user successfully authenticates when trying to access
/browser/dist/admin/admin\*html files (HTTP Basic authentication). Token
is expired after every half an hour, so websocket connection to admin console
must be established within this period.

It should also be possible to do various sorts of tasks such as killing
documents that are open for more than 10 hours etc. See protocol.txt for
various commands. Only tricky thing here is getting the JWT token which can
be obtained as described above.

## Protocol description

See **[protocol.txt](wsd/protocol.txt)** for a description of the protocol to be used over the
websocket.

## Architecture

Please refer to https://sdk.collaboraonline.com/docs/architecture.html

## Enjoy!
