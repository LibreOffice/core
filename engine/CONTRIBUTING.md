# Contributing to Collabora Office
:+1::tada: First off, thanks for taking the time to contribute! :tada::+1:

The following is a set of rules and guidelines for contributing to Collabora Office. Please feel free to propose changes to this document via `git review`.


## Submitting issues

If you have questions about how to install or use Collabora Office, please direct these to our [forum][forum].
If you have issues or questions about Collabora Office development, you may join us on [IRC][irc] or [Telegram][telegram].

### Guidelines
* Please search the existing issues first, it's likely that your issue was already reported or even fixed.
  - Go to the main page of the repository, click "issues" and type any word in the top search/command bar.
  - You can also filter by appending e. g. "state:open" to the search string.
  - More info on [search syntax within github](https://help.github.com/articles/searching-issues)
* __SECURITY__: Report any potential security bug to us following our [security policy](https://github.com/CollaboraOnline/online/security/policy) instead of filing an issue in our bug tracker.

* [Report the issue][report] using one of our templates, they include all the information we need to track down the issue.

Help us to maximize the effort we can spend fixing issues and adding new features, by not reporting duplicate issues.

[report]: https://github.com/CollaboraOnline/online/issues/new/choose
[forum]: https://forum.collaboraonline.com/
[irc]: https://web.libera.chat/?channels=cool-dev
[telegram]: https://t.me/CollaboraOnline

## Contributing to Source Code

Thanks for wanting to contribute source code to Collabora Office. You rock!

1. Clone the repo and set up the git hooks: `cd .git/hooks && ln -s ../../.git-hooks/* ./`
2. Build it (run `autogen.sh` and `make`)
3. Grab one of our newcomer-friendly [easy hacks](https://www.collaboraonline.org/post/easyhacks/)
4. Submit your patch with `git review`

And if you get stuck at any point, just drop by one of our [communication channels](https://www.collaboraonline.org/post/communicate/).

### Sign your work

We use the Developer Certificate of Origin (DCO) as a additional safeguard for the Collabora Office project. This is a well established and widely used mechanism to assure contributors have confirmed their right to license their contribution under the project's license. Please read [README.CONTRIBUTING.md](README.CONTRIBUTING.md). If you can certify it, then just add a line to every git commit message:

````
  Signed-off-by: Random J Developer <random@developer.example.org>
````

Use your real name (sorry, no pseudonyms or anonymous contributions). If you set your `user.name` and `user.email` git configs, you can sign your commit automatically with `git commit -s`.

### Change-Id

We use [change-ids concept](https://gerrit-review.googlesource.com/Documentation/user-changeid.html) in Collabora Office. Install the git hooks (see above) to have the Change-Id added automatically: `cd .git/hooks && ln -s ../../.git-hooks/* ./`

### AI Policy

We consider AI tools that help programming just one more tool that, if used judiciously, can help
developers learn to code.  Volunteers using such AI tools are permitted to get good ideas, but you
are responsible for the resulting code: you should be able to explain each hunk of the diff you have
written based on the generated samples. Submitting patches without understanding their content,
copying AI generated code verbatim, or not testing the result before submitting would create risk,
and waste reviewer time and is forbidden.

## Translations
Please submit translations via [Weblate](https://hosted.weblate.org/projects/collabora-online).
